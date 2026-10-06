# ScriptInterfaces Reversing Progress

## Status

COMPLETE

## Coverage

- Total table entries: 716 (`0x2CC`)
- Completed registered entries: 716
- Last completed index: 715
- Next index: none

## PC Naming Coverage

- Reviewed entries: 716
- Last reviewed index: 715
- Next PC naming backfill index: none
- Renamed: 581
- Already named: 125
- Empty stubs: 6

## Checkpoint

none

## Latest Batch

- Registered indices `707..715`: `Istring__Isempty`, `Pathfinder__Isvectinside`, `Pathfinder__Isposinside`, `Pathfinder__Getsearchposaroundpos`, `Pathfinder__Getposaroundpos`, `Pathfinder__Getentitypos`, `Pathfinder__Getclosestwallpos`, `Pathfinder__Teleportboid`, `Pathfinder__Iswallbetweenpoints`.
- Topology: iOS `0x101602CA8..0x101602CE8`, PS2 `0x85D724..0x85D744`, and PC `0x7F382C..0x7F3850`; all nine applicable entries align consecutively. The three trailing iOS/PS2 null slots are outside the 716-slot PC table.
- PC naming: renamed meaningful targets `0x5A12A0`, `0x5A1350`, `0x5A1390`, `0x5A15D0`, `0x5A1700`, `0x5A13E0`, `0x5A1430`, `0x5A1590`, and `0x5A1830`.
- Signatures: `bool(const char*)`, `bool(v3)`, `bool(float,float,float)`, `v3(v3,float,int,int)`, `v3(v3,float)`, `v3(ZREF)`, `v3(ZREF,v3)`, `void(ZREF,v3)`, and `bool(v3,v3)`.
- Verification: platform topology, signatures, declarations, definitions, table order, CMake registration, checkpoint, and `0x2CC` size assertion passed by the `ScriptInterface` build.

## Latest PC Naming Batch

- Reviewed applicable indices `707..715` during forward registration.
- Result: renamed 9, already named 0, empty stubs 0.
- Verification: live PC table pointers and target bodies were checked; naming coverage reaches the final applicable index `715`.

## Open Decisions

none
