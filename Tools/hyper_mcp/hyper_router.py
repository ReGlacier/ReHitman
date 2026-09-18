"""Unified dependency-free MCP router for multiple IDA Pro instances."""

import argparse
import concurrent.futures
import hmac
import html
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
    "find_references": {
        "description": "Find all code and data references to an address with source context and IDA metadata.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "instance": {"type": "string"},
                "address": {"oneOf": [{"type": "integer"}, {"type": "string"}]},
                "limit": {"type": "integer", "minimum": 1, "maximum": MAX_RESULTS},
            },
            "required": ["instance", "address"],
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
    "rename_global": {
        "description": "Rename a non-function global address in one IDA database with optional compare-and-set protection.",
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
    "search_local_types": {
        "description": "Search Local Types by name in selected IDA instances, or all instances when omitted.",
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
    "add_local_type": {
        "description": "Parse a C declaration and add its named type to IDA Local Types.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "instance": {"type": "string"},
                "declaration": {"type": "string"},
                "replace": {"type": "boolean"},
            },
            "required": ["instance", "declaration"],
            "additionalProperties": False,
        },
    },
    "list_imports": {
        "description": "List imported symbols from one IDA database.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "instance": {"type": "string"},
                "limit": {"type": "integer", "minimum": 1, "maximum": MAX_RESULTS},
            },
            "required": ["instance"],
            "additionalProperties": False,
        },
    },
    "list_exports": {
        "description": "List exported symbols from one IDA database.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "instance": {"type": "string"},
                "limit": {"type": "integer", "minimum": 1, "maximum": MAX_RESULTS},
            },
            "required": ["instance"],
            "additionalProperties": False,
        },
    },
    "set_comment": {
        "description": "Set or clear a regular or repeatable comment at an address.",
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


_INDEX_PAGE_TEMPLATE = """<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Hyper MCP Router — Active Agents</title>
<style>
  :root {
    --bg-top: #fdf3ff;
    --bg-bottom: #e6f4ff;
    --grid: #ffb3ec;
    --sun-a: #ffd6f7;
    --sun-b: #ffe9c7;
    --sun-c: #c9f4ff;
    --neon-pink: #ff6fd8;
    --neon-cyan: #4bd8e0;
    --neon-purple: #b18bff;
    --ink: #4a2a63;
    --ink-soft: #7a5a94;
    --card-bg: rgba(255, 255, 255, 0.62);
    --card-border: rgba(255, 111, 216, 0.55);
  }
  * { box-sizing: border-box; }
  html, body {
    margin: 0;
    min-height: 100%;
    font-family: 'Trebuchet MS', 'Segoe UI', sans-serif;
    color: var(--ink);
  }
  body {
    background: linear-gradient(180deg, var(--bg-top) 0%, var(--bg-bottom) 65%, #d9ecff 100%);
    overflow-x: hidden;
    position: relative;
  }
  .sky {
    position: fixed;
    inset: 0;
    z-index: 0;
    background:
      radial-gradient(circle at 50% 18%, var(--sun-a) 0%, var(--sun-b) 30%, var(--sun-c) 55%, transparent 70%),
      repeating-linear-gradient(180deg, rgba(255,255,255,0) 0px, rgba(255,255,255,0) 38px, rgba(255,182,238,0.35) 39px, rgba(255,182,238,0.35) 40px);
  }
  .sun {
    position: fixed;
    left: 50%;
    top: 10%;
    width: 240px;
    height: 240px;
    transform: translateX(-50%);
    border-radius: 50%;
    background: linear-gradient(180deg, #fff4cf 0%, #ffcfe9 45%, #ffb0e0 70%, #d8a9ff 100%);
    box-shadow: 0 0 70px 10px rgba(255, 180, 230, 0.65), 0 0 140px 40px rgba(190, 160, 255, 0.35);
    z-index: 1;
  }
  .sun::before {
    content: "";
    position: absolute;
    left: 0; right: 0; bottom: 30%;
    height: 6px;
    background: repeating-linear-gradient(180deg, var(--bg-bottom) 0 4px, transparent 4px 12px);
  }
  .horizon-grid {
    position: fixed;
    left: 0; right: 0; bottom: 0;
    height: 42vh;
    z-index: 1;
    background-image:
      linear-gradient(var(--grid) 1px, transparent 1px),
      linear-gradient(90deg, var(--grid) 1px, transparent 1px);
    background-size: 60px 40px, 60px 40px;
    -webkit-mask-image: linear-gradient(180deg, transparent, black 25%);
            mask-image: linear-gradient(180deg, transparent, black 25%);
    transform: perspective(260px) rotateX(55deg);
    transform-origin: bottom;
    opacity: 0.55;
  }
  main {
    position: relative;
    z-index: 2;
    max-width: 880px;
    margin: 0 auto;
    padding: 56px 24px 90px;
  }
  header { text-align: center; margin-bottom: 46px; }
  h1 {
    font-size: 2.6rem;
    letter-spacing: 3px;
    margin: 0 0 8px;
    text-transform: uppercase;
    color: var(--neon-purple);
    text-shadow:
      0 0 6px rgba(255,255,255,0.8),
      0 0 18px var(--neon-pink),
      0 0 34px var(--neon-cyan);
  }
  .subtitle {
    margin: 0;
    color: var(--ink-soft);
    letter-spacing: 2px;
    font-size: 0.85rem;
    text-transform: uppercase;
  }
  .status-line {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    margin-top: 18px;
    padding: 6px 16px;
    border-radius: 999px;
    background: var(--card-bg);
    border: 1px solid var(--card-border);
    font-size: 0.8rem;
    color: var(--ink);
  }
  .dot {
    width: 9px; height: 9px;
    border-radius: 50%;
    background: var(--neon-cyan);
    box-shadow: 0 0 8px var(--neon-cyan);
  }
  .grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(240px, 1fr));
    gap: 22px;
  }
  .card {
    background: var(--card-bg);
    border: 1px solid var(--card-border);
    border-radius: 16px;
    padding: 22px 22px 20px;
    backdrop-filter: blur(6px);
    box-shadow:
      0 0 0 1px rgba(255,255,255,0.4) inset,
      0 8px 24px rgba(177, 139, 255, 0.22),
      0 0 22px rgba(255, 111, 216, 0.18);
    transition: transform 0.25s ease, box-shadow 0.25s ease;
    position: relative;
    overflow: hidden;
  }
  .card::before {
    content: "";
    position: absolute;
    top: 0; left: 0; right: 0;
    height: 3px;
    background: linear-gradient(90deg, var(--neon-pink), var(--neon-purple), var(--neon-cyan));
  }
  .card:hover {
    transform: translateY(-4px);
    box-shadow:
      0 0 0 1px rgba(255,255,255,0.5) inset,
      0 14px 30px rgba(177, 139, 255, 0.3),
      0 0 30px rgba(255, 111, 216, 0.3);
  }
  .card-name {
    font-size: 1.15rem;
    font-weight: 700;
    margin: 0 0 6px;
    color: var(--ink);
    display: flex;
    align-items: center;
    gap: 10px;
  }
  .pulse {
    width: 10px; height: 10px;
    border-radius: 50%;
    background: var(--neon-pink);
    box-shadow: 0 0 10px var(--neon-pink);
    flex: none;
    animation: pulse 2.2s ease-in-out infinite;
  }
  @keyframes pulse {
    0%, 100% { opacity: 1; transform: scale(1); }
    50% { opacity: 0.45; transform: scale(0.8); }
  }
  .card-db {
    margin: 0 0 14px;
    font-size: 0.86rem;
    color: var(--ink-soft);
    word-break: break-word;
  }
  .card-id {
    font-size: 0.7rem;
    letter-spacing: 0.5px;
    color: var(--ink-soft);
    opacity: 0.75;
    font-family: 'Consolas', 'Courier New', monospace;
    word-break: break-all;
  }
  .empty {
    text-align: center;
    padding: 60px 20px;
    color: var(--ink-soft);
    border: 1px dashed var(--card-border);
    border-radius: 16px;
    background: var(--card-bg);
  }
  .empty-title {
    font-size: 1.2rem;
    color: var(--neon-purple);
    text-shadow: 0 0 12px var(--neon-pink);
    margin-bottom: 8px;
    letter-spacing: 1px;
    text-transform: uppercase;
  }
  footer {
    position: relative;
    z-index: 2;
    text-align: center;
    padding: 0 20px 40px;
    color: var(--ink-soft);
    font-size: 0.75rem;
    letter-spacing: 1px;
  }
</style>
</head>
<body>
  <div class="sky"></div>
  <div class="sun"></div>
  <div class="horizon-grid"></div>
  <main>
    <header>
      <h1>Hyper MCP Router</h1>
      <p class="subtitle">Active IDA Agents</p>
      <div class="status-line"><span class="dot"></span>__COUNT_LABEL__</div>
    </header>
    __CONTENT__
  </main>
  <footer>hyper-mcp-router &middot; retrowave console</footer>
</body>
</html>
"""


def _render_index_page():
    entries = sorted(REGISTRY.active(), key=lambda item: item["name"].casefold())
    if entries:
        cards = []
        for entry in entries:
            name = html.escape(str(entry["name"]))
            database = html.escape(str(entry["database"]))
            agent_id = html.escape(str(entry["id"]))
            cards.append(
                """<div class="card">
      <p class="card-name"><span class="pulse"></span>{name}</p>
      <p class="card-db">{database}</p>
      <p class="card-id">{agent_id}</p>
    </div>""".format(name=name, database=database, agent_id=agent_id)
            )
        content = '<div class="grid">\n    ' + "\n    ".join(cards) + "\n  </div>"
        count_label = "%d ONLINE" % len(entries)
    else:
        content = (
            '<div class="empty"><div class="empty-title">No agents connected</div>'
            "Waiting for IDA instances to register&hellip;</div>"
        )
        count_label = "0 ONLINE"
    page = _INDEX_PAGE_TEMPLATE.replace("__CONTENT__", content).replace("__COUNT_LABEL__", count_label)
    return page.encode("utf-8")


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
    "find_references": lambda arguments: _single_instance_call("find_references", arguments, ("address",)),
    "decompile": lambda arguments: _single_instance_call("decompile", arguments, ("address",)),
    "read_memory": lambda arguments: _single_instance_call("read_memory", arguments, ("address", "size")),
    "read_pointer_table": lambda arguments: _single_instance_call(
        "read_pointer_table", arguments, ("address", "count")
    ),
    "rename_function": lambda arguments: _single_instance_call(
        "rename_function", arguments, ("address", "new_name")
    ),
    "rename_global": lambda arguments: _single_instance_call(
        "rename_global", arguments, ("address", "new_name")
    ),
    "set_function_comment": lambda arguments: _single_instance_call(
        "set_function_comment", arguments, ("address", "comment")
    ),
    "search_local_types": lambda arguments: _search("search_local_types", arguments),
    "add_local_type": lambda arguments: _single_instance_call(
        "add_local_type", arguments, ("declaration",)
    ),
    "list_imports": lambda arguments: _single_instance_call("list_imports", arguments, ()),
    "list_exports": lambda arguments: _single_instance_call("list_exports", arguments, ()),
    "set_comment": lambda arguments: _single_instance_call(
        "set_comment", arguments, ("address", "comment")
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

    def _send_html(self, status, body):
        self.send_response(status)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
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
        if self.path == "/":
            self._send_html(200, _render_index_page())
        elif self.path == "/health":
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
