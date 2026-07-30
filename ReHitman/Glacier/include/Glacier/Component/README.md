# Glacier Component System

This directory contains Glacier's component registration and lifetime system.
It is used to describe components, register them before startup, create them in
dependency order, connect components to their managers, and destroy them in the
reverse order.

The system has four main parts:

1. A component class contains the actual service or game logic.
2. A description selects its component group and allocator.
3. A producer registers creation and destruction callbacks.
4. `ZComponentGlobalList` owns the registry and runs the lifecycle.

## Lifecycle Overview

A typical component goes through the following steps:

1. A `ZComponentProducer<T>` is constructed, usually as static registration
   data.
2. The producer adds itself to `ZComponentGlobalList`.
3. `CreateComponents(group)` sorts registered producers by dependencies.
4. The producer creates the component through its `ZComponentDescription`.
5. `InitializeComponent()` adds the component to its typed
   `ZComponentManager<T>`.
6. `DestroyComponents(group)` destroys components in reverse dependency order.

The global list maintains a reference count for every group. A group is only
destroyed when its reference count reaches zero.

## Component Groups

`ZComponentGroupId` is a `uint16_t`, and the runtime currently provides four
groups:

```cpp
inline constexpr ZComponentGroupId ZCOMPONENT_GROUP_COUNT = 4;
```

Valid IDs are therefore `0`, `1`, `2`, and `3`. A
`ZComponentDescription` checks this at compile time.

The base classes currently use these groups:

| Base class | Group | Intended lifetime |
| --- | ---: | --- |
| `ZRuntimeComponentBase` | 0 | Runtime/session components |
| `ZStandardComponentBase` | 0 | Standard runtime components |
| `ZGlobalComponentBase` | 1 | Global application components |

Groups `2` and `3` are available to the engine, but a new use should follow the
actual startup and shutdown sequence. In particular, group `3` is treated as a
system group by `ZComponentGlobalList::CreateComponents`.

## Creating a Component

For common cases, derive from one of the existing base classes and provide a
stable component ID:

```cpp
#include <Glacier/Component/ZRuntimeComponentBase.h>

class ZAudioService : public Glacier::ZRuntimeComponentBase
{
public:
    static Glacier::ZComponentId ComponentId()
    {
        return "ZAudioService";
    }

    void PlaySound();
};
```

`ZRuntimeComponentBase` already supplies the description for group `0` and the
default allocator.

The ID is used for registration and dependency matching. Keep it stable and
unique. ID comparisons are case-insensitive for alphanumeric characters.

## Custom Descriptions

`ZComponent<T, TDescription>` connects a component type to a description. The
description selects a group and an allocator:

```cpp
#include <Glacier/Component/ZComponent.h>
#include <Glacier/Component/ZComponentDefaultAllocator.h>
#include <Glacier/Component/ZComponentDescription.h>

class ZLevelCache
    : public Glacier::ZComponent<
          ZLevelCache,
          Glacier::ZComponentDescription<0, Glacier::ZComponentDefaultAllocator>>
{
public:
    static Glacier::ZComponentId ComponentId()
    {
        return "ZLevelCache";
    }
};
```

An allocator used by `ZComponentDescription` must provide this interface:

```cpp
class MyComponentAllocator
{
public:
    static void* Alloc(uint32_t size);
    static void Free(void* memory);
};
```

`Create<T>()` allocates raw memory and uses placement `new`. `Destroy<T>()`
calls the concrete destructor and returns the memory through the same
allocator.

## Registering Producers

`ZComponentProducer<T>` converts the typed create/destroy operations into the
untyped callbacks stored by `ZComponentProducerData`.

```cpp
#include <Glacier/Component/ZComponentProducer.h>

namespace
{
    char g_AudioParameters[] = "depends ZFileSystem";

    Glacier::ZComponentProducer<ZAudioService> g_AudioProducer(
        "ZAudioServiceImpl",
        g_AudioParameters);
}
```

The first constructor argument is the producer's registration ID. The
component's own `ComponentId()` is also added as a dependency for a dependent
producer. These IDs have different roles and should not be assumed to be
interchangeable.

Creation parameters support two forms:

```text
depends ComponentA ComponentB
parent ParentComponent
```

`depends` declares components that must come first. `parent` marks the producer
as a child and places it after its parent during dependency sorting.

A producer can store at most `ZComponentProducerData::NDEPENDENCIES` (currently
five) dependency IDs.

## Component Managers

Every `ZComponent<T, Description>` calls the matching manager from
`InitializeComponent()`:

```cpp
Glacier::ZComponentManager<T>::Instance().Add(this);
```

`ZComponentManager<T>` keeps an intrusive list of components of that family.
The list links are stored in `ZComponentBase`, so adding a component does not
allocate a separate list node.

The manager itself is registered with `ZComponentManagerProducer<T>`:

```cpp
#include <Glacier/Component/ZComponentManagerProducer.h>

namespace
{
    Glacier::ZComponentManagerProducer<ZAudioService>
        g_AudioManagerProducer(nullptr);
}
```

The manager must exist before a component runs `InitializeComponent()`. Use
dependencies and the established static registration order to preserve this
requirement.

`ZComponentManagerBase` exposes forward and backward traversal:

```cpp
auto& manager = Glacier::ZComponentManager<ZAudioService>::Instance();

for (auto* item = manager.Begin(Glacier::FRONT);
     item != nullptr;
     item = manager.Next(item, Glacier::FRONT))
{
    auto* audio = static_cast<ZAudioService*>(item);
    // Use audio...
}
```

## Singleton Components

`ZComponentSingleton<TComponent, TBase>` adds a typed singleton pointer without
adding instance data to the component layout:

```cpp
#include <Glacier/Component/ZComponentSingleton.h>

class ZTelemetry
    : public Glacier::ZComponentSingleton<
          ZTelemetry,
          Glacier::ZGlobalComponentBase>
{
public:
    static Glacier::ZComponentId ComponentId()
    {
        return "ZTelemetry";
    }

    void RecordEvent();
};

// Put this definition in one .cpp file.
template <>
ZTelemetry* Glacier::ZComponentSingleton<
    ZTelemetry,
    Glacier::ZGlobalComponentBase>::m_pInstance = nullptr;
```

Usage:

```cpp
if (ZTelemetry::Exists())
{
    ZTelemetry::Instance().RecordEvent();
}
```

The constructor verifies that another instance does not exist, and the
destructor clears the pointer. The component description also updates the
singleton pointer when a producer refers to an existing component instance.

Some reverse-engineered engine singletons can specialize `m_pInstance` with an
address owned by the original game instead of `nullptr`.

## Header Reference

| Header | Responsibility |
| --- | --- |
| `Component.h` | Common IDs, group type/count, and lifecycle callback types |
| `Fwds.h` | Forward declarations for the component subsystem |
| `ZComponentBase.h` | ABI-facing polymorphic base and intrusive list links |
| `ZComponent.h` | CRTP component base that connects a component to its description and manager |
| `ZComponentDescription.h` | Group, allocator, typed factory, destructor, and singleton callback generation |
| `ZComponentProducerData.h` | Runtime registration record, dependencies, callbacks, and component pointer |
| `ZComponentProducer.h` | Typed producer for a concrete component |
| `ZComponentGlobalList.h` | Global registry, dependency sorting, group creation, and reverse destruction |
| `ZComponentManagerBase.h` | Intrusive component list and traversal implementation |
| `ZComponentManager.h` | Typed manager singleton for a component family |
| `ZComponentManagerProducer.h` | Producer that registers a typed manager |
| `ZComponentSingleton.h` | Optional CRTP singleton access for a component |
| `ZComponentDefaultAllocator.h` | Default component allocation API |
| `ZGlobalComponentAllocator.h` | Allocation API used by global components |
| `ZRuntimeComponentBase.h` | Ready-to-use group `0` runtime base |
| `ZStandardComponentBase.h` | Ready-to-use group `0` standard base |
| `ZGlobalComponentBase.h` | Ready-to-use group `1` global base |

## Important Rules

- Do not change `ZComponentBase` field order or virtual method order. They are
  part of the recovered engine ABI.
- Use only group IDs smaller than `ZCOMPONENT_GROUP_COUNT`.
- Keep component registration IDs unique.
- Keep dependency IDs alive for as long as the producer exists. String literals
  are the simplest safe choice.
- Do not create a managed component before its `ZComponentManager<T>` exists.
- Define every used `ZComponentSingleton` static pointer exactly once unless an
  existing engine specialization already provides it.
- Use the allocator selected by the description for both creation and
  destruction.
- Keep dependencies acyclic. Dependency sorting does not make dependency cycles
  valid.
