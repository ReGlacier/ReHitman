# Glacier Script Interface

This directory contains the game-specific script interface (SI) of Hitman:
Blood Money: the 716 (`0x2CC`) engine functions that compiled scene scripts
call to drive the game world. Each entry is a free function in namespace
`Glacier` named after its original script-visible owner
(`Zhm3Actor__Sethitpoints`, `Scene__Getdoorstatus`, ...), declared in one
`SI_*.h` header per subsystem and implemented in the matching
`source/SI_*.cpp` file.

The headers serve both sides of the script boundary:

- the game links them as the static library `ScriptInterface` (the
  implementations),
- script DLL projects consume them through the interface target
  `ScriptDllInterface` (signatures only); at runtime the script engine copies
  the filled table into the DLL's import block, so compiled script code
  reaches these implementations by table index.

The engine-side mechanics (DLL attach, `SCRIPTFUNCTIONS`, thread scheduling,
save/load) live in `Glacier/ScriptEngine` - see its README. This module is
only the game API surface.

The module has four main parts:

1. `SI.h` is the umbrella header: it includes every `SI_*.h` and defines
   `SI::ScriptInterfacesTable`, the typed view of all 716 slots in binary
   table order.
2. `SI_*.h` are the per-subsystem declarations (`Zcar`, `Zlink`,
   `Zhm3Actor`, `Item`, `Scene`, ...).
3. `source/SI.cpp` holds the default table and the static initializer that
   publishes it to the engine at process start.
4. `source/SI_*.cpp` hold the implementations, one file per subsystem.

## Big Picture

```text
Process start (static init)               Scene script DLL attach
        |                                       |
        v                                       v
ScriptInterfacesInitializer        ScriptEngine::AttachSceneScripts()
 copies ScriptInterfacesDefault          copies Glacier::ScriptInterfaces
 into Glacier::ScriptInterfaces[0x2CC]   into the DLL's Z_ScriptImports block
        |                                       |
        +------------------+------------------+
                           |
                           v
        compiled script code calls Z_ScriptImports[i](args...)
                           |
                           v
        Glacier::{Group}__{Method}(args...)     (source/SI_*.cpp)
                           |
                           v
        Glacier engine / BloodMoney game objects (ZHM3Actor, ZGEOM, ...)
```

## The Table

`SI::ScriptInterfacesTable` (`SI.h`) is the typed layout of the script
interface table: exactly `0x2CC` function pointers in the original PC order,
enforced by

```cpp
static_assert(sizeof(SI::ScriptInterfacesTable) == sizeof(void*) * 0x2CC);
```

`source/SI.cpp` initializes `ScriptInterfacesDefault` with every
implementation address (each entry is commented with its decimal slot index)
and registers it at startup by copying the pointers into the engine's global
`ScriptInterfaces` array (`Glacier/ScriptEngine/Globals.h`):

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

When a scene's script DLL is attached, the script engine copies this array
verbatim into the DLL's `Z_ScriptImports` export (ordinal 4); compiled script
code then calls entries by index. Slot order is therefore a hard binary
contract with already-compiled scripts. Verified anchors: slot 0 is
`Zcar__Moveto`, slot 715 is `Pathfinder__Iswallbetweenpoints`.

Naming and typing rules:

- Functions keep the original `{Group}__{Method}` names (double underscore);
  a trailing underscore escapes C++ keywords (`Zcar__Continue_`).
- Signatures use the script-facing types from
  `Glacier/ScriptEngine/Common.h` (`ZREF`, `v3`, `anim`, `ZSC_EVENT`,
  `sCover`, ...) so the call ABI matches the original build.
- Reversing status: all 716 slots are registered and named (see
  `PROGRESS.md`); a few entries are intentional empty stubs preserved for
  table topology.

## CMake Targets

| Target | Kind | Purpose |
| --- | --- | --- |
| `ScriptDllInterface` | INTERFACE | Exposes `include/` to script DLL consumers (signatures only) |
| `ScriptInterface` | STATIC | The implementations; publicly links `ScriptDllInterface` and `G1::Core` (`G1::TestCore` in test builds), and uses BloodMoney headers |

On MSVC, `ScriptInterface` propagates `/WHOLEARCHIVE` to its consumers so the
static initializer (and with it the whole table) survives linking even though
nothing references it directly.

## Header Reference

| Header | Responsibility |
| --- | --- |
| `SI.h` | Umbrella header + `SI::ScriptInterfacesTable` (all 716 typed slots in table order) |
| `SI_ZCar.h` | `Zcar__*`: car path driving, speed, seats, Hitman collision |
| `SI_ZLink.h` | `Zlink__*`: animation playback, segments, meta keys, anim destination |
| `SI_ZHM3Actor.h` | `Zhm3Actor__*`: actor vision/needs/tension, weapons, inventory, clothes/disguise, speech, ragdoll, elevators |
| `SI_ZActor.h` | `Zactor__*`: actor movement, path reservations, life state, look-at |
| `SI_Zgeom.h` | `Zgeom__*`: transforms, distances, angles, rooms, controllers, activation |
| `SI_Zhm3Boid.h` | `Zhm3Boid__*`: flocking behaviors (separation/cohesion/seek/flee/waypoints) |
| `SI_Zhm3Securitycamera.h` | `Zhm3Securitycamera__*`: fuse box, radio channel, foot point |
| `SI_Item.h` | `Item__*`: item pickup/hands/containers, ownership, edibles, concealment |
| `SI_Hm3Weapon.h` | `Hm3Weapon__*`: weapon template/type/ammo queries |
| `SI_Coverlist.h` | `Coverlist__*`: AI cover search and cover/shoot position extraction |
| `SI_Dialog.h` | `Dialog__*`: dialog start/stop, cameras, skip control |
| `SI_Fightcontroller.h` | `Fightcontroller__*`: combat targeting, hunting, cover fire |
| `SI_Friskbox.h` | `Friskbox__*`: frisk zones, guards, suitcases |
| `SI_Guardquartercontroller.h` | `Guardquartercontroller__*`: guard tasks, weapon storage, body bag boxes |
| `SI_Actorcommunication.h` | `Actorcommunication__*`: radio channels, ranged/global actor events |
| `SI_Silevelcontrol.h` | `Silevelcontrol__*`: mission objectives, Hitman state, notoriety, difficulty |
| `SI_Holevelcontrol.h` | `Holevelcontrol__*`: hideout-specific level control |
| `SI_M04Levelcontrol.h` | `M04Levelcontrol__*`: mission 04 specific queries |
| `SI_M11Levelcontrol.h` | `M11Levelcontrol__*`: mission 11 specifics (projector, karaoke, crowd) |
| `SI_M12Levelcontrol.h` / `SI_M12LevelcontrolExtra.h` | `M12Levelcontrol__*`: mission 12 specifics (high noon, bomb) |
| `SI_Bloodtrail.h` | `Bloodtrail__*`: blood trail queries and deactivation |
| `SI_Cigs.h` | `Cigs__*`: cigarette pack get/free |
| `SI_Zlist.h` | `Zlist__*`: geom list container operations and event broadcast |
| `SI_Debugfunctions.h` | `Debugfunctions__*` / `Add*decay`: debug render lines and text |
| `SI_Physics.h` | `Physics__*`: ragdoll bone release, velocity |
| `SI_Engine.h` | `Print`/`Error`/`Warning`/`Zscassert`, `Engine__*`: random, time, scene values, locale |
| `SI_Audio.h` | `Audio__*`: 2D/3D sound playback, volume, notify targets |
| `SI_Math.h` | `Math__*`: `v3` math helpers, `Sqrtf` |
| `SI_Istring.h` | `Istring__*`: name compare/contains, empty string check |
| `SI_Pathfinder.h` | `Pathfinder__*`: navmesh inside tests, around-pos search, wall checks, boid teleport |
| `SI_Scene.h` | `Scene__*`: object position/scale, box queries, doors, noise level |
| `SI_Osd.h` | `Osd__*`: on-screen info and objective tick-off marks |
| `SI_Pathfollower.h` | `Pathfollower__*`: waypoint list selection and following |
| `SI_Poscontroller.h` | `Poscontroller__*`: actor position controller registry |
| `SI_Zusepoint.h` | `Zusepoint__*`: use point lock/probability/relocate |

## Important Rules

- Never reorder, insert, or remove entries in `ScriptInterfacesTable` or
  `ScriptInterfacesDefault`; the slot index is the ABI. A newly reversed
  function must land at its original index, and the `static_assert` on
  `0x2CC` must keep passing.
- Keep declarations in `SI_*.h`, definitions in `source/SI_*.cpp`, and the
  table field types exactly in sync; the table copies addresses blindly and
  there is no type check against the script side.
- Do not move the functions out of namespace `Glacier` (they are game-side
  implementations); only the typed table struct lives in namespace `SI`.
- Do not change signatures to host types: parameters must stay the
  script-facing Glacier types (`ZREF`, `v3`, `anim`, ...) so the call ABI
  matches compiled scripts.
- If an implementation needs an entity that is not reversed yet, leave
  `// TODO: Finish this place after {ClassName} will be reversed` at the call
  site instead of inventing stubs or new layout.
- Keep the `/WHOLEARCHIVE` link propagation intact; without it the linker
  strips the static initializer and the script DLL receives an empty import
  table.
