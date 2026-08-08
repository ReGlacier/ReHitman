"""Unified dependency-free MCP router for multiple IDA Pro instances."""

import argparse
import concurrent.futures
import hmac
import json
import queue
import secrets
import socket
import threading
import time
import uuid
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer


MAX_REQUEST_SIZE = 16 * 1024 * 1024
MAX_RESULTS = 200
REQUEST_TIMEOUT = 120
INSTANCE_TTL = REQUEST_TIMEOUT + 30
POLL_WAIT = 30
DISCOVERY_PORT = 8764


TOOLS = {
    "list_instances": {
        "description": "List active named IDA instances and their databases.",
        "inputSchema": {"type": "object", "properties": {}, "additionalProperties": False},
    },
    "search_functions": {
        "description": "Search function names in selected IDA instances, or all instances when omitted.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "query": {"type": "string"},
                "instances": {"type": "array", "items": {"type": "string"}},
                "limit": {"type": "integer", "minimum": 1, "maximum": MAX_RESULTS},
            },
            "required": ["query"],
            "additionalProperties": False,
        },
    },
    "search_globals": {
        "description": "Search global names in selected IDA instances, or all instances when omitted.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "query": {"type": "string"},
                "instances": {"type": "array", "items": {"type": "string"}},
                "limit": {"type": "integer", "minimum": 1, "maximum": MAX_RESULTS},
            },
            "required": ["query"],
            "additionalProperties": False,
        },
    },
    "decompile": {
        "description": "Return Hex-Rays pseudocode from one named IDA instance.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "instance": {"type": "string"},
                "address": {"oneOf": [{"type": "integer"}, {"type": "string"}]},
            },
            "required": ["instance", "address"],
            "additionalProperties": False,
        },
    },
    "read_memory": {
        "description": "Read database bytes from one named IDA instance as hex or base64.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "instance": {"type": "string"},
                "address": {"oneOf": [{"type": "integer"}, {"type": "string"}]},
                "size": {"type": "integer", "minimum": 1, "maximum": 65536},
                "encoding": {"type": "string", "enum": ["hex", "base64"]},
            },
            "required": ["instance", "address", "size"],
            "additionalProperties": False,
        },
    },
    "read_pointer_table": {
        "description": "Read a pointer table with names and function metadata from one IDA database.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "instance": {"type": "string"},
                "address": {"oneOf": [{"type": "integer"}, {"type": "string"}]},
                "count": {"type": "integer", "minimum": 1, "maximum": 4096},
                "pointer_size": {"type": "integer", "enum": [2, 4, 8]},
                "stride": {"type": "integer", "minimum": 1},
                "pointer_offset": {"type": "integer", "minimum": 0},
            },
            "required": ["instance", "address", "count"],
            "additionalProperties": False,
        },
    },
    "rename_function": {
        "description": "Rename an existing function in one IDA database with optional compare-and-set protection.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "instance": {"type": "string"},
                "address": {"oneOf": [{"type": "integer"}, {"type": "string"}]},
                "new_name": {"type": "string"},
                "expected_name": {"type": "string"},
                "force": {"type": "boolean"},
            },
            "required": ["instance", "address", "new_name"],
            "additionalProperties": False,
        },
    },
    "set_function_comment": {
        "description": "Set or clear an existing function's comment with optional compare-and-set protection.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "instance": {"type": "string"},
                "address": {"oneOf": [{"type": "integer"}, {"type": "string"}]},
                "comment": {"type": "string"},
                "repeatable": {"type": "boolean"},
                "expected_comment": {"type": "string"},
                "force": {"type": "boolean"},
            },
            "required": ["instance", "address", "comment"],
            "additionalProperties": False,
        },
    },
    "search_and_decompile": {
        "description": "Search a function in multiple builds and decompile each unambiguous match.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "query": {"type": "string"},
                "instances": {"type": "array", "items": {"type": "string"}},
            },
            "required": ["query"],
            "additionalProperties": False,
        },
    },
}


class AgentState:
    def __init__(self, data):
        self.id = data["id"]
        self.name = data["name"]
        self.database = data["database"]
        self.session = secrets.token_urlsafe(32)
        self.commands = queue.Queue()
        self.pending = {}
        self.lock = threading.Lock()
        self.last_seen = time.monotonic()

    def as_entry(self):
        return {
            "id": self.id,
            "name": self.name,
            "database": self.database,
            "last_seen": self.last_seen,
            "state": self,
        }

    def call(self, method, arguments):
        request_id = str(uuid.uuid4())
        completed = threading.Event()
        response = {}
        with self.lock:
            self.pending[request_id] = (completed, response)
        self.commands.put({"request_id": request_id, "method": method, "arguments": arguments})
        if not completed.wait(REQUEST_TIMEOUT):
            with self.lock:
                self.pending.pop(request_id, None)
            raise TimeoutError("agent did not answer within %d seconds" % REQUEST_TIMEOUT)
        return response["payload"]

    def complete(self, request_id, payload):
        with self.lock:
            pending = self.pending.pop(request_id, None)
        if pending is None:
            return False
        completed, response = pending
        response["payload"] = payload
        completed.set()
        return True


class Registry:
    def __init__(self):
        self.lock = threading.Lock()
        self.entries = {}

    def register(self, data):
        required = ("id", "name", "database")
        if not all(isinstance(data.get(key), str) and data[key] for key in required):
            raise ValueError("registration requires id, name and database")
        state = AgentState(data)
        with self.lock:
            old = self.entries.get(data["id"])
            self.entries[data["id"]] = state
        if old is not None:
            with old.lock:
                pending = list(old.pending.values())
                old.pending.clear()
            for completed, response in pending:
                response["payload"] = {"ok": False, "error": "agent reconnected"}
                completed.set()
        return state

    def authenticate(self, agent_id, session):
        with self.lock:
            state = self.entries.get(agent_id)
        if state is None or not hmac.compare_digest(state.session, str(session)):
            raise ValueError("invalid or expired agent session")
        state.last_seen = time.monotonic()
        return state

    def active(self):
        now = time.monotonic()
        with self.lock:
            self.entries = {key: value for key, value in self.entries.items() if now - value.last_seen <= INSTANCE_TTL}
            return [value.as_entry() for value in self.entries.values()]

    def resolve(self, names=None):
        entries = self.active()
        by_name = {}
        for entry in entries:
            by_name.setdefault(entry["name"].casefold(), []).append(entry)
        if names is None:
            names = [entry["name"] for entry in entries]
        if not isinstance(names, list) or not all(isinstance(name, str) for name in names):
            raise ValueError("instances must be an array of names")
        resolved = []
        errors = {}
        for name in names:
            matches = by_name.get(name.casefold(), [])
            if not matches:
                errors[name] = {"status": "instance_offline"}
            elif len(matches) > 1:
                errors[name] = {"status": "ambiguous_instance", "count": len(matches)}
            else:
                resolved.append(matches[0])
        return resolved, errors


REGISTRY = Registry()


def _agent_call(entry, method, arguments):
    try:
        payload = entry["state"].call(method, arguments)
    except (OSError, TimeoutError) as exc:
        return {"status": "instance_offline", "error": str(exc)}
    if not payload.get("ok"):
        message = payload.get("error", "unknown agent error")
        status = "hexrays_unavailable" if "decompiler is unavailable" in message else "error"
        return {"status": status, "error": message}
    return {"status": "found", "result": payload.get("result")}


def _parallel_call(entries, method, arguments):
    results = {}
    if not entries:
        return results
    with concurrent.futures.ThreadPoolExecutor(max_workers=len(entries)) as executor:
        futures = {executor.submit(_agent_call, entry, method, arguments): entry for entry in entries}
        for future, entry in futures.items():
            result = future.result()
            if method.startswith("search_") and result["status"] == "found" and not result["result"]:
                result["status"] = "not_found"
            results[entry["name"]] = result
    return results


def _list_instances(arguments):
    return {
        "instances": [
            {"name": entry["name"], "database": entry["database"], "id": entry["id"]}
            for entry in sorted(REGISTRY.active(), key=lambda item: item["name"].casefold())
        ]
    }


def _search(method, arguments):
    if not str(arguments.get("query", "")).strip():
        raise ValueError("query must not be empty")
    entries, errors = REGISTRY.resolve(arguments.get("instances"))
    agent_arguments = {"query": arguments["query"], "limit": arguments.get("limit", 50)}
    errors.update(_parallel_call(entries, method, agent_arguments))
    return {"query": arguments["query"], "results": errors}


def _single_instance_call(method, arguments, required):
    missing = [name for name in required if name not in arguments]
    if "instance" not in arguments:
        missing.insert(0, "instance")
    if missing:
        raise ValueError("missing required fields: %s" % ", ".join(missing))
    entries, errors = REGISTRY.resolve([arguments["instance"]])
    if errors:
        return {"instance": arguments["instance"], "result": errors[arguments["instance"]]}
    agent_arguments = {key: value for key, value in arguments.items() if key != "instance"}
    return {"instance": entries[0]["name"], "result": _agent_call(entries[0], method, agent_arguments)}


def _search_and_decompile(arguments):
    query = str(arguments.get("query", "")).strip()
    if not query:
        raise ValueError("query must not be empty")
    entries, results = REGISTRY.resolve(arguments.get("instances"))

    searches = _parallel_call(entries, "search_functions", {"query": query, "limit": MAX_RESULTS})
    decompile_jobs = []
    for entry in entries:
        search = searches[entry["name"]]
        if search["status"] != "found":
            results[entry["name"]] = search
            continue
        matches = search["result"]
        exact = [match for match in matches if match["name"].casefold() == query.casefold()]
        candidates = exact or matches
        if len(candidates) != 1:
            results[entry["name"]] = {"status": "ambiguous", "matches": matches}
            continue
        decompile_jobs.append((entry, candidates[0]))

    if decompile_jobs:
        with concurrent.futures.ThreadPoolExecutor(max_workers=len(decompile_jobs)) as executor:
            futures = {
                executor.submit(_agent_call, entry, "decompile", {"address": match["address"]}): (entry, match)
                for entry, match in decompile_jobs
            }
            for future, (entry, match) in futures.items():
                result = future.result()
                if result["status"] == "found":
                    results[entry["name"]] = {"status": "found", **result["result"]}
                else:
                    results[entry["name"]] = {**result, "match": match}
    return {"query": query, "results": results}


HANDLERS = {
    "list_instances": _list_instances,
    "search_functions": lambda arguments: _search("search_functions", arguments),
    "search_globals": lambda arguments: _search("search_globals", arguments),
    "decompile": lambda arguments: _single_instance_call("decompile", arguments, ("address",)),
    "read_memory": lambda arguments: _single_instance_call("read_memory", arguments, ("address", "size")),
    "read_pointer_table": lambda arguments: _single_instance_call(
        "read_pointer_table", arguments, ("address", "count")
    ),
    "rename_function": lambda arguments: _single_instance_call(
        "rename_function", arguments, ("address", "new_name")
    ),
    "set_function_comment": lambda arguments: _single_instance_call(
        "set_function_comment", arguments, ("address", "comment")
    ),
    "search_and_decompile": _search_and_decompile,
}


class RouterHandler(BaseHTTPRequestHandler):
    server_version = "HyperMcpRouter/3.0"

    def log_message(self, fmt, *args):
        print("[Hyper MCP Router] " + fmt % args)

    def _send(self, status, payload):
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Access-Control-Allow-Origin", "null")
        self.end_headers()
        self.wfile.write(body)

    def _read_json(self):
        length = int(self.headers.get("Content-Length", "0"))
        if length < 1 or length > MAX_REQUEST_SIZE:
            raise ValueError("invalid request size")
        return json.loads(self.rfile.read(length).decode("utf-8"))

    def _authorized(self):
        token = self.server.auth_token
        return not token or hmac.compare_digest(self.headers.get("Authorization", ""), "Bearer " + token)

    def do_GET(self):
        if self.path == "/health":
            self._send(200, _list_instances({}))
        else:
            self._send(404, {"error": "not found"})

    def do_POST(self):
        try:
            if self.path == "/agent/register":
                state = REGISTRY.register(self._read_json())
                print("[Hyper MCP Router] Agent '%s' connected from %s" % (state.name, self.client_address[0]))
                self._send(200, {"session": state.session})
                return
            if self.path == "/agent/poll":
                data = self._read_json()
                state = REGISTRY.authenticate(data.get("id"), data.get("session"))
                try:
                    command = state.commands.get(timeout=POLL_WAIT)
                except queue.Empty:
                    command = None
                state.last_seen = time.monotonic()
                self._send(200, {"command": command})
                return
            if self.path == "/agent/result":
                data = self._read_json()
                state = REGISTRY.authenticate(data.get("id"), data.get("session"))
                payload = {"ok": bool(data.get("ok"))}
                if payload["ok"]:
                    payload["result"] = data.get("result")
                else:
                    payload["error"] = data.get("error", "unknown agent error")
                accepted = state.complete(data.get("request_id"), payload)
                self._send(200, {"accepted": accepted})
                return
            if self.path != "/mcp":
                self._send(404, {"error": "not found"})
                return
            if not self._authorized():
                self._send(401, {"error": "invalid or missing bearer token"})
                return
            response = self._dispatch(self._read_json())
            if response is None:
                self.send_response(202)
                self.send_header("Content-Length", "0")
                self.end_headers()
            else:
                self._send(200, response)
        except (ValueError, json.JSONDecodeError) as exc:
            self._send(400, self._error(None, -32700, str(exc)))
        except Exception as exc:
            self._send(500, self._error(None, -32603, str(exc)))

    @staticmethod
    def _error(request_id, code, message):
        return {"jsonrpc": "2.0", "id": request_id, "error": {"code": code, "message": message}}

    def _dispatch(self, request):
        if not isinstance(request, dict) or request.get("jsonrpc") != "2.0":
            return self._error(request.get("id") if isinstance(request, dict) else None, -32600, "Invalid request")
        request_id = request.get("id")
        if request_id is None:
            return None
        method = request.get("method")
        if method == "initialize":
            return {
                "jsonrpc": "2.0",
                "id": request_id,
                "result": {
                    "protocolVersion": "2025-03-26",
                    "capabilities": {"tools": {"listChanged": False}},
                    "serverInfo": {"name": "hyper-mcp-router", "version": "3.0.0"},
                },
            }
        if method == "ping":
            return {"jsonrpc": "2.0", "id": request_id, "result": {}}
        if method == "tools/list":
            tools = [{"name": name, **definition} for name, definition in TOOLS.items()]
            return {"jsonrpc": "2.0", "id": request_id, "result": {"tools": tools}}
        if method == "tools/call":
            params = request.get("params") or {}
            handler = HANDLERS.get(params.get("name"))
            if handler is None:
                return self._error(request_id, -32602, "Unknown tool")
            try:
                data = handler(params.get("arguments") or {})
                result = {
                    "content": [{"type": "text", "text": json.dumps(data, ensure_ascii=False, indent=2)}],
                    "structuredContent": data,
                }
            except Exception as exc:
                result = {"content": [{"type": "text", "text": str(exc)}], "isError": True}
            return {"jsonrpc": "2.0", "id": request_id, "result": result}
        return self._error(request_id, -32601, "Method not found")


class RouterServer(ThreadingHTTPServer):
    daemon_threads = True
    allow_reuse_address = True

    def __init__(self, address, token):
        self.auth_token = token
        super().__init__(address, RouterHandler)


def discovery_loop(stop_event, http_port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.bind(("", DISCOVERY_PORT))
    sock.settimeout(1)
    print("[Hyper MCP Router] UDP discovery listening on port %d" % DISCOVERY_PORT)
    try:
        while not stop_event.is_set():
            try:
                data, address = sock.recvfrom(4096)
                request = json.loads(data.decode("utf-8"))
                if request.get("protocol") != "hyper-mcp" or request.get("version") != 1:
                    continue
                print("[Hyper MCP Router] Discovery from %s:%d" % address)
                offer = {"protocol": "hyper-mcp", "version": 1, "nonce": request.get("nonce"), "port": http_port}
                payload = json.dumps(offer).encode("utf-8")
                sock.sendto(payload, address)
                print("[Hyper MCP Router] Discovery offer sent unicast to %s:%d" % address)

                # Some bridged VM adapters pass guest broadcasts but lose the
                # host's unicast reply. Repeat the nonce-matched offer through
                # the directed /24 broadcast while the source port is open.
                octets = address[0].split(".")
                if len(octets) == 4:
                    broadcast = ".".join(octets[:3] + ["255"])
                    try:
                        sock.sendto(payload, (broadcast, address[1]))
                        print("[Hyper MCP Router] Discovery offer sent broadcast to %s:%d" % (broadcast, address[1]))
                    except OSError:
                        pass
            except socket.timeout:
                continue
            except (OSError, ValueError, json.JSONDecodeError):
                continue
    finally:
        sock.close()


def main():
    parser = argparse.ArgumentParser(description="Route one MCP endpoint to multiple IDA instances")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--token", default="")
    args = parser.parse_args()
    server = RouterServer((args.host, args.port), args.token)
    stop_event = threading.Event()
    discovery_thread = threading.Thread(
        target=discovery_loop, args=(stop_event, args.port), name="hyper-mcp-discovery", daemon=True
    )
    discovery_thread.start()
    print("[Hyper MCP Router] MCP listening on http://%s:%d/mcp" % (args.host, args.port))
    print("[Hyper MCP Router] Start IDA agents on the same Host-only network")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        stop_event.set()
        server.server_close()


if __name__ == "__main__":
    main()
