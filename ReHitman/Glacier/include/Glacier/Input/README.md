# Glacier Input System

This directory contains Glacier's input abstraction layer. It maps platform input
devices into a common device/control model used by gameplay, actions, UI, and
debug tooling.

The system has three main layers:

1. `SysInput::ZInterface` defines the global input manager API.
2. `ZSysInput` stores registered devices and implements device lookup, name
   mapping, activation, and handler installation.
3. `SysInput::ZDevice` and `ZInputDevice` define the common per-device API for
   digital controls, analog controls, sampling history, and event handlers.

Platform-specific Wintel devices derive from the shared device classes:

| Device | Base class | Responsibility |
| --- | --- | --- |
| `ZMouseWintel` | `ZMouseDevice` | DirectInput mouse state, buttons, wheel, and relative motion |
| `ZKeyboardWintel` | `ZKeyboardDevice` | Buffered DirectInput keyboard events, key repeat, and text conversion |
| `ZGameControlWintel` | `ZGameControlDevice` | DirectInput game controller axes, buttons, dead-zone filtering, and polling |

## Device Model

Each device exposes digital and analog controls by integer id. Control names are
stored in `ZInputDevice::CtrlInfo` sentinel-terminated tables and are used by
mapping APIs such as `MapDigital()` and `MapAnalog()`.

```cpp
int deviceId = SysInput::instance->GetDeviceIdByName("kb");
int controlId = 0;

if (SysInput::instance->MapDigital("kb", "space", &deviceId, &controlId))
{
    const int pressed = SysInput::instance->Digital(deviceId, controlId);
}
```

Digital state changes are sampled with a short four-entry `TIMETYPE` history.
Analog controls keep the current value and a motion value. Mouse motion is
accumulated as relative movement, while game-controller analog motion stores the
difference between the new and previous absolute value.

## Device Names

`ZSysInput::GetDeviceName()` returns short canonical names for primary devices:

| Type | First device | Additional devices |
| --- | --- | --- |
| Mouse | `ms` | device-provided name |
| Keyboard | `kb` | device-provided name |
| Game controller | `gc` | `gc1`, `gc2`, ... |

`GetDeviceIdByName()` accepts aliases such as `mouse`, `ms`, `keyboard`, `kb`,
`gc`, and `gcN`, and falls back to matching the device's `GetName()` string.

## Handlers

Devices have two handler slots. `InstallHandler()` stores callbacks in the first
free slot, and `UninstallHandler()` clears matching handlers. Keyboard Wintel
uses these callbacks for converted key events and repeat handling.

```cpp
int OnInput(unsigned int keyCode, int mode, void* userData);

device->InstallHandler(OnInput, context);
```

## Wintel DirectInput

The Wintel backend uses DirectInput 8. `ZSysInputWintel` creates the global
`IDirectInput8A` instance, enumerates devices, creates the corresponding Wintel
device classes, updates them, and can acquire or unacquire all devices.

DirectInput headers and libraries are platform dependencies of the Glacier CMake
target. Generic input tests should exercise `ZInputDevice` and `ZSysInput`
behavior without constructing Wintel devices.

## Important Rules

- Do not reorder virtual methods; the vtable order is matched to the original PC
  binary.
- Do not change field order or add padding casually; device sizes and offsets are
  verified for binary compatibility.
- Keep Wintel/DirectInput logic out of generic input tests.
- Prefer `const char*` for control and device names; control tables store string
  literals.
- Leave TODO call sites for input classes or platform details that are not fully
  reversed yet.
