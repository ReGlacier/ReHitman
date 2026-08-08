"""IDA Pro 9.2 agent for the unified Hyper MCP router.

Copy this file to IDA's plugins directory. The agent binds an automatically
discovers hyper_router.py and keeps an outbound command channel open.
"""

import base64
import ipaddress
import json
import os
import socket
import threading
import traceback
import urllib.error
import urllib.request
import uuid

import ida_bytes
import ida_funcs
import ida_hexrays
import ida_ida
import ida_idaapi
import ida_kernwin
import ida_name
import ida_nalt
import ida_netnode
import idaapi
import idautils


PLUGIN_NAME = "IDA MCP Agent"
NODE_NAME = "$ ida_mcp_agent"
ROUTER_URL = os.environ.get("IDA_MCP_ROUTER", "")
DISCOVERY_PORT = 8764
DISCOVERY_INTERVAL = 5
DISCOVERY_TIMEOUT = 2
POLL_TIMEOUT = 40
MAX_RESULTS = 200
MAX_MEMORY_READ = 64 * 1024
MAX_TABLE_ENTRIES = 4096
NODE_ID_SLOT = 0
NODE_NAME_SLOT = 1


def _decode_node_string(value):
    if isinstance(value, bytes):
        return value.rstrip(b"\0").decode("utf-8")
    return value or ""


def _node_read_string(node, slot, legacy_key):
    value = _decode_node_string(node.supval_idx8(slot, ida_netnode.stag))
    if value:
        return value
    return _decode_node_string(node.hashstr(legacy_key))


def _node_write_string(node, slot, value):
    data = value.encode("utf-8")
    node.supset_idx8(slot, data, ida_netnode.stag)
    saved = _decode_node_string(node.supval_idx8(slot, ida_netnode.stag))
    if saved != value:
        raise RuntimeError("failed to save agent identity in the IDA database")


def _parse_address(value):
    if isinstance(value, int):
        return value
    if not isinstance(value, str) or not value.strip():
        raise ValueError("address must be an integer, a hex string, or a symbol name")
    text = value.strip()
    try:
        return int(text, 0)
    except ValueError:
        ea = ida_name.get_name_ea(ida_idaapi.BADADDR, text)
        if ea == ida_idaapi.BADADDR:
            raise ValueError("unknown address or symbol: %s" % text)
        return ea


def _bounded_limit(arguments):
    limit = int(arguments.get("limit", 50))
    if limit < 1 or limit > MAX_RESULTS:
        raise ValueError("limit must be between 1 and %d" % MAX_RESULTS)
    return limit


def _search_functions(arguments):
    query = str(arguments.get("query", "")).casefold()
    if not query:
        raise ValueError("query must not be empty")
    limit = _bounded_limit(arguments)
    results = []
    for ea in idautils.Functions():
        name = ida_funcs.get_func_name(ea)
        if query in name.casefold():
            function = ida_funcs.get_func(ea)
            results.append(
                {
                    "address": "0x%X" % ea,
                    "name": name,
                    "end_address": "0x%X" % function.end_ea,
                    "size": function.end_ea - function.start_ea,
                }
            )
            if len(results) >= limit:
                break
    return results


def _search_globals(arguments):
    query = str(arguments.get("query", "")).casefold()
    if not query:
        raise ValueError("query must not be empty")
    limit = _bounded_limit(arguments)
    results = []
    for ea, name in idautils.Names():
        if query not in name.casefold() or ida_funcs.get_func(ea) is not None:
            continue
        results.append({"address": "0x%X" % ea, "name": name})
        if len(results) >= limit:
            break
    return results


def _decompile(arguments):
    if "address" not in arguments:
        raise ValueError("address is required")
    if not ida_hexrays.init_hexrays_plugin():
        raise RuntimeError("Hex-Rays decompiler is unavailable for this database")
    ea = _parse_address(arguments["address"])
    function = ida_funcs.get_func(ea)
    if function is None:
        raise ValueError("no function contains address 0x%X" % ea)
    cfunc = ida_hexrays.decompile(function.start_ea)
    if cfunc is None:
        raise RuntimeError("Hex-Rays could not decompile 0x%X" % function.start_ea)
    return {
        "address": "0x%X" % function.start_ea,
        "name": ida_funcs.get_func_name(function.start_ea),
        "code": str(cfunc),
    }


def _read_memory(arguments):
    if "address" not in arguments or "size" not in arguments:
        raise ValueError("address and size are required")
    ea = _parse_address(arguments["address"])
    size = int(arguments["size"])
    if size < 1 or size > MAX_MEMORY_READ:
        raise ValueError("size must be between 1 and %d" % MAX_MEMORY_READ)
    data = ida_bytes.get_bytes(ea, size)
    if data is None or len(data) != size:
        raise ValueError("could not read %d bytes at 0x%X" % (size, ea))
    encoding = arguments.get("encoding", "hex")
    if encoding == "hex":
        value = data.hex()
    elif encoding == "base64":
        value = base64.b64encode(data).decode("ascii")
    else:
        raise ValueError("encoding must be hex or base64")
    return {"address": "0x%X" % ea, "size": size, "encoding": encoding, "data": value}


def _read_pointer_table(arguments):
    if "address" not in arguments or "count" not in arguments:
        raise ValueError("address and count are required")
    ea = _parse_address(arguments["address"])
    count = int(arguments["count"])
    if count < 1 or count > MAX_TABLE_ENTRIES:
        raise ValueError("count must be between 1 and %d" % MAX_TABLE_ENTRIES)
    pointer_size = int(arguments.get("pointer_size", 8 if ida_ida.inf_is_64bit() else 4))
    if pointer_size not in (2, 4, 8):
        raise ValueError("pointer_size must be 2, 4, or 8")
    stride = int(arguments.get("stride", pointer_size))
    pointer_offset = int(arguments.get("pointer_offset", 0))
    if stride < pointer_size or pointer_offset < 0 or pointer_offset + pointer_size > stride:
        raise ValueError("pointer must fit within a positive table stride")
    byteorder = "big" if ida_ida.inf_is_be() else "little"
    entries = []
    for index in range(count):
        slot_ea = ea + index * stride + pointer_offset
        raw = ida_bytes.get_bytes(slot_ea, pointer_size)
        if raw is None or len(raw) != pointer_size:
            entries.append({"index": index, "slot_address": "0x%X" % slot_ea, "status": "unreadable"})
            continue
        target = int.from_bytes(raw, byteorder=byteorder, signed=False)
        function = ida_funcs.get_func(target)
        entries.append(
            {
                "index": index,
                "slot_address": "0x%X" % slot_ea,
                "target": "0x%X" % target,
                "name": ida_name.get_name(target) or "",
                "is_function": function is not None and function.start_ea == target,
                "containing_function": "0x%X" % function.start_ea if function is not None else None,
            }
        )
    return {
        "address": "0x%X" % ea,
        "count": count,
        "pointer_size": pointer_size,
        "stride": stride,
        "pointer_offset": pointer_offset,
        "byteorder": byteorder,
        "entries": entries,
    }


def _function_at(arguments):
    if "address" not in arguments:
        raise ValueError("address is required")
    ea = _parse_address(arguments["address"])
    function = ida_funcs.get_func(ea)
    if function is None:
        raise ValueError("no function contains address 0x%X" % ea)
    return function


def _rename_function(arguments):
    function = _function_at(arguments)
    new_name = str(arguments.get("new_name", "")).strip()
    if not new_name:
        raise ValueError("new_name must not be empty")
    current_name = ida_funcs.get_func_name(function.start_ea)
    expected_name = arguments.get("expected_name")
    if expected_name is not None and current_name != expected_name:
        raise ValueError("current name is %s, expected %s" % (current_name, expected_name))
    has_user_name = ida_bytes.has_user_name(ida_bytes.get_flags(function.start_ea))
    if has_user_name and expected_name is None and not arguments.get("force", False) and current_name != new_name:
        raise ValueError("refusing to replace user name %s without expected_name or force" % current_name)
    if current_name != new_name and not ida_name.set_name(function.start_ea, new_name, ida_name.SN_CHECK):
        raise ValueError("IDA rejected function name: %s" % new_name)
    return {"address": "0x%X" % function.start_ea, "old_name": current_name, "name": new_name}


def _set_function_comment(arguments):
    function = _function_at(arguments)
    comment = str(arguments.get("comment", ""))
    repeatable = bool(arguments.get("repeatable", False))
    current = ida_funcs.get_func_cmt(function, repeatable) or ""
    expected = arguments.get("expected_comment")
    if expected is not None and current != expected:
        raise ValueError("current comment does not match expected_comment")
    if current and current != comment and expected is None and not arguments.get("force", False):
        raise ValueError("refusing to replace existing comment without expected_comment or force")
    if current != comment and not ida_funcs.set_func_cmt(function, comment, repeatable):
        raise RuntimeError("IDA failed to set the function comment")
    return {
        "address": "0x%X" % function.start_ea,
        "name": ida_funcs.get_func_name(function.start_ea),
        "old_comment": current,
        "comment": comment,
        "repeatable": repeatable,
    }


METHODS = {
    "search_functions": _search_functions,
    "search_globals": _search_globals,
    "decompile": _decompile,
    "read_memory": _read_memory,
    "read_pointer_table": _read_pointer_table,
    "rename_function": _rename_function,
    "set_function_comment": _set_function_comment,
}


def _run_in_ida(function, write=False):
    result = []

    def invoke():
        try:
            result.append((True, function()))
        except Exception as exc:
            result.append((False, exc))
        return 1

    write_flag = getattr(ida_kernwin, "MFF_WRITE", ida_kernwin.MFF_FAST)
    ida_kernwin.execute_sync(invoke, write_flag if write else ida_kernwin.MFF_READ)
    if not result:
        raise RuntimeError("IDA did not execute the request")
    if not result[0][0]:
        raise result[0][1]
    return result[0][1]


class IdaMcpPlugin(idaapi.plugin_t):
    flags = idaapi.PLUGIN_FIX
    comment = "Register this IDA database with the unified MCP router"
    help = "Run this plugin from the menu to rename the instance"
    wanted_name = PLUGIN_NAME
    wanted_hotkey = ""

    def _load_identity(self):
        self.database_path = ida_nalt.get_input_file_path()
        if not self.database_path:
            raise RuntimeError("IDA database is not loaded yet")
        self.node = ida_netnode.netnode(NODE_NAME, 0, True)
        self.instance_id = _node_read_string(self.node, NODE_ID_SLOT, "id")
        if not self.instance_id:
            self.instance_id = str(uuid.uuid4())
        _node_write_string(self.node, NODE_ID_SLOT, self.instance_id)
        self.instance_name = _node_read_string(self.node, NODE_NAME_SLOT, "name")
        if not self.instance_name:
            filename = os.path.splitext(os.path.basename(self.database_path))[0]
            self.instance_name = ida_kernwin.ask_str(filename or "IDA", 0, "Name this IDA instance (PC, iOS, PS2, ...)")
            if not self.instance_name:
                self.instance_name = filename or self.instance_id[:8]
        _node_write_string(self.node, NODE_NAME_SLOT, self.instance_name)

    def _registration(self):
        return {
            "id": self.instance_id,
            "name": self.instance_name,
            "database": self.database_path,
        }

    @staticmethod
    def _post(url, payload, timeout):
        request = urllib.request.Request(
            url,
            data=json.dumps(payload).encode("utf-8"),
            headers={"Content-Type": "application/json"},
            method="POST",
        )
        # Agent traffic is always local/private and must not inherit a host or
        # corporate HTTP proxy from the guest environment.
        opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
        with opener.open(request, timeout=timeout) as response:
            return json.loads(response.read().decode("utf-8"))

    @staticmethod
    def _discovery_targets():
        targets = {"255.255.255.255"}
        try:
            addresses = socket.gethostbyname_ex(socket.gethostname())[2]
        except OSError:
            addresses = []
        for address in addresses:
            try:
                ip = ipaddress.ip_address(address)
                if not ip.is_loopback and ip.version == 4:
                    network = ipaddress.ip_network(address + "/24", strict=False)
                    targets.add(str(network.broadcast_address))
                    targets.add(str(network.network_address + 1))
                    targets.add(str(network.network_address + 2))
            except ValueError:
                continue
        return sorted(targets)

    def _discover_router(self):
        if ROUTER_URL:
            return ROUTER_URL.rstrip("/")
        nonce = uuid.uuid4().hex
        message = json.dumps({"protocol": "hyper-mcp", "version": 1, "nonce": nonce}).encode("utf-8")
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            sock.settimeout(DISCOVERY_TIMEOUT)
            sock.bind(("", 0))
            targets = self._discovery_targets()
            self.discovery_attempts += 1
            if self.discovery_attempts == 1 or self.discovery_attempts % 12 == 0:
                print("[%s] Discovery attempt %d via %s UDP/%d" % (
                    PLUGIN_NAME, self.discovery_attempts, ", ".join(targets), DISCOVERY_PORT
                ))
            for target in targets:
                try:
                    sock.sendto(message, (target, DISCOVERY_PORT))
                except OSError:
                    continue
            try:
                while not self.stop_event.is_set():
                    data, address = sock.recvfrom(4096)
                    offer = json.loads(data.decode("utf-8"))
                    if offer.get("protocol") == "hyper-mcp" and offer.get("nonce") == nonce:
                        router_url = "http://%s:%d" % (address[0], int(offer["port"]))
                        print("[%s] Discovered router at %s" % (PLUGIN_NAME, router_url))
                        return router_url
            except socket.timeout:
                pass

            # Some VPN/firewall filters pass guest-to-host UDP but drop the
            # unicast reply. Probe the conventional host addresses directly.
            direct_targets = [target for target in targets if not target.endswith(".255") and target != "255.255.255.255"]
            for target in direct_targets:
                router_url = "http://%s:8765" % target
                try:
                    connection = socket.create_connection((target, 8765), timeout=1)
                    connection.close()
                    print("[%s] Found router directly at %s" % (PLUGIN_NAME, router_url))
                    return router_url
                except OSError:
                    continue
        except (OSError, ValueError, json.JSONDecodeError):
            return None
        finally:
            sock.close()

    def _execute_command(self, command):
        handler = METHODS.get(command.get("method"))
        if handler is None:
            return {"ok": False, "error": "unknown method"}
        try:
            arguments = command.get("arguments") or {}
            write = command.get("method") in ("rename_function", "set_function_comment")
            data = _run_in_ida(lambda: handler(arguments), write)
            return {"ok": True, "result": data}
        except Exception as exc:
            return {"ok": False, "error": str(exc)}

    def _connection_loop(self):
        connected_url = None
        while not self.stop_event.is_set():
            router_url = self._discover_router()
            if not router_url:
                self.stop_event.wait(DISCOVERY_INTERVAL)
                continue
            try:
                connected_url = router_url
                registration = self._post(router_url + "/agent/register", self._registration(), 5)
                session = registration["session"]
                print("[%s] '%s' connected to %s" % (PLUGIN_NAME, self.instance_name, router_url))
                while not self.stop_event.is_set() and not self.reconnect_event.is_set():
                    command = self._post(
                        router_url + "/agent/poll",
                        {"id": self.instance_id, "session": session},
                        POLL_TIMEOUT,
                    ).get("command")
                    if command is None:
                        continue
                    result = self._execute_command(command)
                    result.update({"id": self.instance_id, "session": session, "request_id": command["request_id"]})
                    self._post(router_url + "/agent/result", result, 10)
            except (OSError, KeyError, ValueError, urllib.error.URLError, json.JSONDecodeError) as exc:
                print("[%s] Could not use %s: %s; rediscovering router" % (PLUGIN_NAME, connected_url, exc))
            connected_url = None
            self.reconnect_event.clear()
            self.stop_event.wait(DISCOVERY_INTERVAL)

    def init(self):
        if not idaapi.get_path(idaapi.PATH_TYPE_IDB):
            return idaapi.PLUGIN_SKIP
        try:
            self._load_identity()
            self.stop_event = threading.Event()
            self.reconnect_event = threading.Event()
            self.discovery_attempts = 0
            self.connection_thread = threading.Thread(target=self._connection_loop, name="hyper-mcp-channel", daemon=True)
            self.connection_thread.start()
            print("[%s] '%s' searching for a Hyper MCP router" % (PLUGIN_NAME, self.instance_name))
            return idaapi.PLUGIN_KEEP
        except Exception as exc:
            print("[%s] Failed to start: %s" % (PLUGIN_NAME, exc))
            traceback.print_exc()
            return idaapi.PLUGIN_SKIP

    def run(self, arg):
        name = ida_kernwin.ask_str(self.instance_name, 0, "Rename this IDA MCP instance")
        if name and name != self.instance_name:
            self.instance_name = name
            _node_write_string(self.node, NODE_NAME_SLOT, name)
            self.reconnect_event.set()
            print("[%s] Instance renamed to '%s'" % (PLUGIN_NAME, name))

    def term(self):
        if hasattr(self, "stop_event"):
            self.stop_event.set()


def PLUGIN_ENTRY():
    return IdaMcpPlugin()
