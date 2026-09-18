# Glacier Action System

This directory contains Glacier's action binding system. It maps text action
configuration into a tree of input expressions, then exposes those expressions
through lightweight handles used by gameplay code.

The system has four main parts:

1. `Action::Interface` defines the public action manager API.
2. `ZActionManager` owns the action tree and parses binding scripts.
3. `ZActionMapTree` stores one expression node and evaluates digital or analog state.
4. `Action::ZHandle` caches lookups by action name and forwards runtime queries.

## Basic Usage

Bindings are written as assignments. Blocks group actions by name and become tree
nodes under the root action map:

```cpp
#include <Glacier/Action/ZActionManager.h>

Glacier::ZActionManager actions;

actions.AddBindings(R"(
Debug={
    ToggleFreeCam=tap(kb,k);
    Pause=tap(kb,p);
    EndDeathSequence=tap(kb,z);
    CycleInventory=tap(kb,x);
};
)");

Glacier::ZActionMapTree* pause = actions.GetMapping("Pause");
```

Input expressions reference `SysInput` device names and control names. During
parsing, digital expressions use `SysInput::ZInterface::MapDigital()`, while
analog and relative expressions use `MapAnalog()`.

## Binding Syntax

The parser consumes prefix-form expressions from `ZTokenStream`. A binding can be
a constant, a reference to another action, an input predicate, a comparison, an
arithmetic expression, or a sequence.

```text
ActionName = tap(kb,space);
Run       = hold(kb,lshift);
MoveX     = ana(gc,x);
AimDelta  = rel(ms,x);
Either    = | tap(kb,z) tap(kb,x);
Combo     = seq(tap(kb,a),tap(kb,b),tap(kb,c));
```

Supported digital predicates are `hold`, `tap`, `release`, `downedge`,
`fasttap`, `doubleclick`, `holddown`, `clickhold`, and `press`.

Analog values use `ana(device,control)` for absolute state and
`rel(device,control)` for motion. Numeric operators are prefix-form `+`, `-`,
and `*`; comparisons are `>` and `<`; boolean operators are `&` and `|`.

## ZActionManager

`ZActionManager` owns the root `ZActionMapTree` and parses binding text into
children below that root. Construction also applies every registered
`Action::ZStaticBinds` block.

```cpp
Glacier::ZActionManager manager;

if (!manager.AddBindings("Jump=tap(kb,space);"))
{
    printf("bad action binding\n");
}

Glacier::ZActionMapTree* jump = manager.GetMapping("Jump");
```

`LoadBindings()` loads a file through `g_pSysFile` and then calls
`AddBindings()`. Parse failures are non-fatal for file loading but are reported
to debug output.

`GetMapping()` searches recursively by node name. `SeqNr()` increments after a
successful `AddBindings()` so cached handles can refresh stale pointers.

## ZActionMapTree

`ZActionMapTree` is both a named action and an expression node. The `EType` value
selects how the node evaluates:

| Type range | Meaning |
| --- | --- |
| `eGET` | Forward evaluation to another named action |
| `eCONSTANT` | Return a fixed float or boolean value |
| `eHOLD` .. `ePRESS` | Digital input predicates |
| `eAND`, `eOR` | Boolean aggregation over child expressions |
| `eGT`, `eLT` | Analog comparisons over two child expressions |
| `eSEQUENCE` | Ordered digital sequence over child expressions |
| `eANALOG`, `eRELATIVE` | Absolute analog state or analog motion |
| `ePLUS`, `eMINUS`, `eMULT` | Arithmetic over two child expressions |

Digital and analog evaluation depends on `SysInput::instance` and the current
engine time in `g_pSysInterface->m_fRealTime`. Digital predicates use the input
device's current digital state and four-entry digital history.

```cpp
if (jump && jump->Digital())
{
    int device = jump->ActivatedBy();
}
```

`Override()` replaces device/control ids for input expression nodes and marks
them with `eOVERRIDDEN`. `RevertToDefault()` restores original parsed ids.
`SetDevice()` retargets mapped controls to another device of the same input type
and recurses through children.

## Action Handles

`Action::ZHandle` is a small runtime reference by action name. It caches the
resolved map pointer and the manager sequence number, then refreshes when the
binding tree changes.

```cpp
Glacier::Action::ZHandle pause("Pause");

if (pause.Digital())
{
    // Pause was activated.
}
```

When controller joining is enabled through `SetJoinControllers(true, ...)`,
`ZHandle::Digital()` and `ZHandle::Analog()` iterate over game controller
devices and retarget the mapped expression before evaluating it.

## Static Binds

`Action::ZStaticBinds` stores binding text in a global linked list. A
`ZActionManager` constructor walks that list and applies every registered bind
block.

```cpp
static Glacier::Action::ZStaticBinds s_DebugBinds(R"(
Debug={
    ToggleFreeCam=tap(kb,k);
};
)");
```

Static binds are useful for module-local defaults that should exist before any
external action configuration is loaded.

## Header Reference

| Header | Responsibility |
| --- | --- |
| `ActionInterface.h` | Public `Action` namespace API, `ZHandle`, static binds, and abstract manager interface |
| `ZActionManager.h` | Binding parser, action tree owner, lookup, loading, and runtime manager state |
| `ZActionMapTree.h` | Expression node layout, evaluation, override/default handling, and display helpers |

## Important Rules

- Do not change field order or virtual method order. `ZActionManager` and
  `ZActionMapTree` have verified binary-compatible sizes.
- Keep binding names stable. `ZHandle` stores a raw action name pointer and uses
  it for future lookups.
- Ensure `SysInput::instance` exists before parsing bindings that contain
  `tap`, `hold`, `ana`, `rel`, or other input expressions.
- Ensure `g_pSysInterface` exists before evaluating digital or analog actions,
  because time-based predicates read `m_fRealTime`.
- Use `RevertToDefault()` after temporary overrides if runtime remapping should
  return to parsed device/control ids.
- `GetKeyName()` returns logical control names; `GetSystemKeyName()` returns
  platform/system display names where available.
