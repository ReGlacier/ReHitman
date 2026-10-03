# ScriptInterface — Implementation Specs & Type Contracts

Source of truth: `ReHitman/ScriptInterface/` (module README + PROGRESS.md are
the upstream references; this doc pins them in the ScriptCS context base).
The module is the **game-side implementation half** of the 716-entry script
interface table. It is also the header source that script DLL projects
(`ScriptCS`) consume for signatures.

## What it is

716 (`0x2CC`) engine functions that compiled scene scripts call to drive the
game world. Each entry is a free function in namespace `Glacier` named after
its original script-visible owner (`Zhm3Actor__Sethitpoints`,
`Scene__Getdoorstatus`, ...), declared in one `SI_*.h` per subsystem,
implemented in the matching `source/SI_*.cpp`.

Two consumers, one set of headers:

- the **game** links them as the static library `ScriptInterface`
  (implementations + table publisher),
- **script DLL projects** consume signatures through the INTERFACE target
  `ScriptDllInterface`; at engine attach the filled `ScriptInterfaces` global
  is copied into the DLL's ordinal-4 import block, so compiled script code
  reaches the implementations **by table index**, not by symbol.

## File layout & build targets

```
ScriptInterface/
├── CMakeLists.txt
├── README.md            # upstream module doc
├── PROGRESS.md          # reversing status (COMPLETE: 716/716 registered+named)
├── include/SI/
│   ├── SI.h             # umbrella: includes every SI_*.h + SI::ScriptInterfacesTable
│   └── SI_*.h           # per-subsystem declarations (namespace Glacier)
└── source/
    ├── SI.cpp           # ScriptInterfacesDefault + ScriptInterfacesInitializer
    └── SI_*.cpp         # implementations, one file per subsystem
```

CMake targets (`ScriptInterface/CMakeLists.txt`):

| Target | Kind | Contract |
| --- | --- | --- |
| `ScriptDllInterface` | INTERFACE | exposes `include/` to script DLL consumers (signatures only) |
| `ScriptInterface` | STATIC | the 39 `SI_*.cpp` implementations; PUBLIC include `include/` + `BloodMoney/include`, PRIVATE `Glacier/include`; links `ScriptDllInterface` + `G1::Core` |

MSVC: `ScriptInterface` propagates `/WHOLEARCHIVE` to consumers — without it
the linker strips `ScriptInterfacesInitializerInstance` and attached DLLs get an
empty import table. **Keep this intact.**

## The table (hard binary ABI)

`SI::ScriptInterfacesTable` (`include/SI/SI.h`, namespace `SI`,
`using namespace Glacier;` inside): exactly `0x2CC` function pointers in
original PC slot order, enforced by

```cpp
static_assert(sizeof(SI::ScriptInterfacesTable) == sizeof(void*) * 0x2CC);
```

(declared in `source/SI.cpp`, next to the default table.)

`source/SI.cpp` defines

```cpp
static const SI::ScriptInterfacesTable ScriptInterfacesDefault = {
    &Zcar__Moveto,            // 0
    ...                       // every entry commented with its decimal index
    &Pathfinder__Iswallbetweenpoints, // 715
};
```

and publishes it at process start:

```cpp
struct ScriptInterfacesInitializer
{
    ScriptInterfacesInitializer()
    {
        auto* source = reinterpret_cast<const void* const*>(&ScriptInterfacesDefault);
        for (int i = 0; i < 0x2CC; ++i)
            ScriptInterfaces[i] = const_cast<void*>(source[i]);
    }
};
static const ScriptInterfacesInitializer ScriptInterfacesInitializerInstance;
```

`ScriptInterfaces` is the engine global from
`Glacier/ScriptEngine/Globals.h` (PC `0x007F2D20`). On DLL attach the engine
copies it verbatim into the DLL's `ScriptImports` block
(see SCRIPT_ENGINE.md / SCRIPTCS_CONTRACT.md).

Verified anchors: slot 0 `Zcar__Moveto`, slot 715
`Pathfinder__Iswallbetweenpoints`; PC table pointers checked during naming
batches (e.g. PC `0x7F382C..0x7F3850` for slots ~707..715).

## Naming & typing rules

- Keep original `{Group}__{Method}` names (double underscore). A trailing
  underscore escapes C++ keywords: `Zcar__Continue_`,
  `Silevelcontrol__Remove_Existing_Eventcam`.
- **All** functions live in namespace `Glacier`; only the typed table struct
  lives in namespace `SI`.
- Parameters/returns must use script-facing types (`Glacier/ScriptEngine/Common.h`
  + `Glacier/ZREF.h`): `ZREF`, `v3`, `anim`, `ZSC_EVENT`, `sCover`, `eMeta`,
  `eAudioEvent/Type`, `Hitman::*` enums from BloodMoney headers... Do **not**
  substitute host types — the call ABI must match compiled scripts.
- Declarations in `SI_*.h`, definitions in `source/SI_*.cpp`, table field types
  identical — the table copies addresses blindly; nothing type-checks the
  script side against the implementation.
- Reversal TODO convention at call sites:
  `// TODO: Finish me` / `// TODO: Finish me after <Class> reversed`.

## Reversing status (as of this snapshot)

- 716/716 entries registered and named; 581 renamed from placeholders, 125
  pre-named, **6 intentional empty stubs** kept for table topology.
- Latest batch: slots 707–715 (Istring/Pathfinder); see `PROGRESS.md`.
- Many `source/SI_*.cpp` bodies are `// TODO: Finish me` stubs (e.g. the whole
  of `SI_ZCar.cpp`) — signatures still bind into the table; completion status
  per function does not affect ABI.

## Domain grouping (header reference)

Group prefix → header (`include/SI/`) → responsibility:

| Group(s) in table | Header | Responsibility |
| --- | --- | --- |
| `Zcar__*` (0..13) | `SI_ZCar.h` | car path driving, speed, seats, Hitman collision |
| `Zlink__*` (14..30) | `SI_ZLink.h` | anim playback, segments, meta keys, anim destination |
| `Zhm3Actor__*` | `SI_ZHM3Actor.h` | vision/needs/tension, weapons, inventory, clothes/disguise, speech, ragdoll, elevators |
| `Zactor__*` | `SI_ZActor.h` | movement, path reservations, life state, look-at |
| `Zgeom__*` | `SI_Zgeom.h` | transforms, distances, rooms, controllers, activation |
| `Zhm3Boid__*` | `SI_Zhm3Boid.h` | flocking (separation/cohesion/seek/flee/waypoints) |
| `Zhm3Securitycamera__*` | `SI_Zhm3Securitycamera.h` | fuse box, radio channel, foot point |
| `Item__*` | `SI_Item.h` | pickup/hands/containers, ownership, edibles, concealment |
| `Hm3Weapon__*` | `SI_Hm3Weapon.h` | weapon template/type/ammo queries |
| `Coverlist__*` | `SI_Coverlist.h` | AI cover search, cover/shoot position extraction |
| `Dialog__*` | `SI_Dialog.h` | dialog start/stop, cameras, skip control |
| `Fightcontroller__*` | `SI_Fightcontroller.h` | combat targeting, hunting, cover fire |
| `Friskbox__*` | `SI_Friskbox.h` | frisk zones, guards, suitcases |
| `Guardquartercontroller__*` | `SI_Guardquartercontroller.h` | guard tasks, weapon storage, body-bag boxes |
| `Actorcommunication__*` | `SI_Actorcommunication.h` | radio channels, ranged/global actor events |
| `Silevelcontrol__*` | `SI_Silevelcontrol.h` | objectives, Hitman state, notoriety, difficulty, event cameras |
| `Holevelcontrol__*` | `SI_Holevelcontrol.h` | hideout-specific level control (the Hideout DLL's own group) |
| `M04Levelcontrol__*` / `M11Levelcontrol__*` / `M12Levelcontrol(Extra)__*` | matching headers | per-mission queries |
| `Bloodtrail__*`, `Cigs__*`, `Zlist__*` | matching headers | blood trails, cigarette packs, geom lists + event broadcast |
| `Debugfunctions__*`, `Add*decay` | `SI_Debugfunctions.h` | debug render lines/text |
| `Physics__*` | `SI_Physics.h` | ragdoll bone release, velocity |
| `Print/Error/Warning/Zscassert`, `Engine__*` | `SI_Engine.h` | random, time, scene COM values, locale |
| `Audio__*` | `SI_Audio.h` | 2D/3D sound, volume, notify targets |
| `Math__*` | `SI_Math.h` | v3 helpers, `Sqrtf` |
| `Istring__*` (705..707) | `SI_Istring.h` | name compare/contains, empty check |
| `Pathfinder__*` (708..715) | `SI_Pathfinder.h` | navmesh inside tests, around-pos search, wall checks, boid teleport |
| `Scene__*` | `SI_Scene.h` | object position/scale, box queries, doors, noise level |
| `Osd__*` | `SI_Osd.h` | on-screen info, objective tick-off marks |
| `Pathfollower__*` | `SI_Pathfollower.h` | waypoint list selection/following |
| `Poscontroller__*` | `SI_Poscontroller.h` | actor position-controller registry |
| `Zusepoint__*` | `SI_Zusepoint.h` | use-point lock/probability/relocate |

Shared helpers (not table entries): `SI_Common.h/.cpp` —
`GetGeom`, `GetValidIKLnkObj`, `GetValidLnkObj`, `GetValidItem`,
`SlipItemFromHand`; these validate `ZREF`→pointer conversions and warn like
the originals.

## Table-order anchors (from SI.cpp comments, verified 2026-09-17)

```
0    Zcar__Moveto
14   Zlink__Stopallanims
31   Zhm3Actor__Setblinking
694  Math__Vdist
705  Istring__Isnameequal
707  Istring__Isempty
708  Pathfinder__Isvectinside
715  Pathfinder__Iswallbetweenpoints
```

Full index ⇄ name mapping: SI.h field order == SI.cpp comment numbering ==
runtime slot. `Docs/SI_table_map.txt` (repo root) is the historical dump.

## Rules for new/modified work

1. Never reorder/insert/remove `ScriptInterfacesTable`/`ScriptInterfacesDefault`
   entries; a newly reversed function lands at its original index; the `0x2CC`
   `static_assert` must keep passing.
2. Keep declarations / definitions / table field types in sync.
3. No host-stdlib containers in Glacier-facing code (STLport rules from
   root `AGENTS.md`); no ordinary `new/delete`; `ZASSERT(false)` not
   `__debugbreak()`.
4. Unreversed dependency → `// TODO: Finish me ...` at the call site, with the
   expected decompiled call as a comment below when useful.
5. MSVC `/WHOLEARCHIVE` propagation stays.
6. ScriptDLL (`ScriptCS`) compiles against these headers through
   `ScriptDllInterface` — adding an SI function requires adding only the
   declaration for signature-consumers; implementations are game-side only.
