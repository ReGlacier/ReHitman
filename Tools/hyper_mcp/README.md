# Hyper IDA MCP - Cooperative MCP commander

This setup combines any number of IDA Pro 9.2 processes behind one MCP URL:

```text
Model -> router on host <- outbound channels from PC, iOS and PS2 IDA agents
```

Both scripts use only Python's standard library:

- `hyper_mcp.py`: IDA plugin/agent. Each IDB stores its own name and UUID.
- `hyper_router.py`: unified MCP server and live instance registry.

## Installation

1. Copy `hyper_mcp.py` to `%APPDATA%\Hex-Rays\IDA Pro\plugins\hyper_mcp.py` in the
   guest, or to the `plugins` directory under the IDA installation.
2. Keep `hyper_router.py` anywhere convenient on the physical host.
3. Set the VM network adapter to **Host-only** or **Bridged** on a trusted
   private network.
4. Start the router on the host:

```powershell
python hyper_router.py
```

5. Start all required IDA processes normally.

On the first load of each database, the plugin asks for its short name, such as
`PC`, `iOS`, or `PS2`. That name is stored inside the IDB and is not requested
again. Use `Edit -> Plugins -> IDA MCP Agent` to rename an instance later.

The agents broadcast a discovery packet on UDP port `8764`. The router replies
with its TCP port both directly and through the subnet broadcast, then each
agent opens an outbound long-poll HTTP connection to the address that answered.
The duplicate reply helps bridged VM adapters that pass guest broadcasts but
lose the host's unicast UDP response. Agents do not listen on any TCP port, and
the guest requires no inbound TCP firewall rule. No IP address, port, or
environment-variable configuration is needed.

Start order does not matter. If the router stops, the network disconnects, or a
VM is paused and resumed, the agent drops its stale session and broadcasts
again every five seconds until the router returns.

Instance names must be unique among open IDA processes. A duplicate name is
reported as `ambiguous_instance` instead of routing a request unpredictably.

## MCP connection

Configure one MCP Streamable HTTP server on the host itself:

```text
http://127.0.0.1:8765/mcp
```

Only the router listens on the network. IDA agents initiate outbound requests
and cannot be reached directly from the host. The default setup has no MCP
authentication; keep the VM adapter in isolated Host-only mode.

Optional router settings:

```powershell
python hyper_router.py --host 0.0.0.0 --port 8765 --token optional-secret
```

When `--token` is used, configure the MCP client's HTTP header:

```text
Authorization: Bearer optional-secret
```

If Windows asks about network access for Python on the host, allow only Private
networks. The host firewall must permit inbound discovery and agent traffic
from the VMware Host-only subnet:

```powershell
New-NetFirewallRule -DisplayName "Hyper MCP agents" `
  -Direction Inbound -Action Allow -Protocol TCP -LocalPort 8765 `
  -RemoteAddress 192.168.137.0/24 -Profile Any
New-NetFirewallRule -DisplayName "Hyper MCP discovery" `
  -Direction Inbound -Action Allow -Protocol UDP -LocalPort 8764 `
  -RemoteAddress 192.168.137.0/24 -Profile Any
```

Replace `192.168.137.0/24` with the subnet of the Host-only adapter. For an
explicit fallback that bypasses UDP discovery, set `IDA_MCP_ROUTER` in the
guest to the host URL, for example `http://192.168.137.1:8765`, before starting
IDA.

If an agent remains at `searching for a Hyper MCP router`, compare the IP
addresses shown by `ipconfig` on the host's `VMware Network Adapter VMnet1` and
inside the guest. They must be in the same subnet. The agent log prints every
discovery destination; the router prints `Discovery from ...` when a packet
arrives. No router message means VMware adapter mode or the host UDP firewall
is blocking discovery. A router discovery message without a following agent
connection usually means the guest firewall is blocking the UDP reply or TCP
8765.

## MCP tools

- `list_instances`: list active IDA names, database paths, and persistent IDs.
- `search_functions`: search selected builds or all active builds in parallel.
- `search_globals`: search global names in selected builds or all builds.
- `find_references`: find code and data references to an address, including xref
  type, disassembly, segment, source name, and containing-function context.
- `decompile`: decompile an address or exact symbol in one named build.
- `read_memory`: read up to 64 KiB of database bytes as hex or base64.
- `read_pointer_table`: read up to 4096 pointer-table entries with target
  names and function metadata. Supports custom pointer size, structure stride,
  and pointer offset.
- `rename_function`: rename an existing function with overwrite protection.
- `rename_global`: rename a non-function global address with overwrite protection.
- `set_function_comment`: set, replace, or clear a function comment with
  overwrite protection.
- `search_local_types`: search Local Types by name in selected builds or all builds.
- `add_local_type`: parse a C declaration and add its named type to Local Types.
- `list_imports`: list imported symbols from one IDA database.
- `list_exports`: list exported symbols from one IDA database.
- `set_comment`: set, replace, or clear a comment at any address with overwrite protection.
- `search_and_decompile`: search multiple builds and decompile unambiguous
  matches in one request.

For example, read three `ScriptInterfaces` tables independently:

```json
{
  "instance": "PC",
  "address": "ScriptInterfaces",
  "count": 715
}
```

```json
{
  "instance": "iOS",
  "address": "_ScriptInterfaces",
  "count": 716
}
```

```json
{
  "instance": "PS2",
  "address": "ScriptInterfaces",
  "count": 688
}
```

Use these requests with `read_pointer_table`. `pointer_size` defaults to the
database bitness. For an array of structures, specify `stride` and
`pointer_offset`; for a plain pointer array they default to the pointer size and
zero. Each entry contains its index, slot address, target address, current name,
and whether the target is already a function. The model can then decompile
selected targets from each build instead of transferring every function at
once.

Renames, comments, and Local Types are the IDB modifications currently exposed.
Prefer compare-and-set arguments so a
stale model request cannot overwrite newer manual work:

```json
{
  "instance": "PC",
  "address": "0x401000",
  "expected_name": "sub_401000",
  "new_name": "ZEventBase::FindLocation"
}
```

Find references to any named or numeric address in one build:

```json
{
  "instance": "PC",
  "address": "ScriptInterfaces",
  "limit": 200
}
```

The result describes the target and each code or data xref. It reports the full
xref count in `total`; `truncated` is true when only the first `limit` entries
are returned.

Rename a global using compare-and-set protection:

```json
{
  "instance": "PC",
  "address": "0x8A1234",
  "expected_name": "dword_8A1234",
  "new_name": "g_CurrentLevel"
}
```

```json
{
  "instance": "PC",
  "address": "0x401000",
  "expected_comment": "",
  "comment": "Matched to PS2 ScriptInterfaces[143]"
}
```

An existing user-defined function/global name or non-empty comment is not
replaced unless its current value is supplied as
`expected_name`/`expected_comment`, or `force` is explicitly set to `true`.
Passing an empty `comment` clears the selected comment.

Add a named Local Type through IDA's declaration parser:

```json
{
  "instance": "PC",
  "declaration": "struct ZExample { int value; };",
  "replace": false
}
```

`search_local_types` returns matching names, ordinals, and printable
declarations. `list_imports` and `list_exports` return symbol names, addresses,
ordinals, and module/forwarder metadata where IDA provides it. `set_comment`
accepts symbol names or numeric addresses and supports the same
`expected_comment`/`force` protection as function comments.

Example cross-build request:

```json
{
  "query": "ZEventBase::FindLocation",
  "instances": ["PC", "PS2"]
}
```

Use it with `search_and_decompile`. A normal response can contain independent
statuses for each build:

```json
{
  "query": "ZEventBase::FindLocation",
  "results": {
    "PC": {
      "status": "not_found"
    },
    "PS2": {
      "status": "found",
      "address": "0x123456",
      "name": "ZEventBase::FindLocation",
      "code": "..."
    }
  }
}
```

Possible statuses include `found`, `not_found`, `ambiguous`,
`instance_offline`, `ambiguous_instance`, `hexrays_unavailable`, and `error`.
Failure in one IDA does not discard successful results from other builds.

IDA API operations are marshalled onto each IDA process's main thread. HTTP
workers never access an IDB directly. An instance disappears from the router
approximately 15 seconds after its IDA process closes.

## Multiple VMs later

Additional VMs on a network that forwards UDP broadcast can discover the same
router without changes. Across routed networks, set `IDA_MCP_ROUTER` explicitly
or add a site-specific discovery relay. Agent connections remain outbound, so
raw IDA ports never need to be exposed.
