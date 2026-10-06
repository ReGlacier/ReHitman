---
name: zgeom-class-reversing
description: Reverse one PC ZGEOM-derived Glacier class from its vtable and source files, annotate the vtable, recover RTTI globals and properties, reconstruct overrides, and update the class header and implementation.
compatibility: opencode with Hyper MCP IDA tools, PC/iOS/PS2 comparison instances, and the ReHitman source/build tools available
metadata:
  project: ReHitman
  workflow: reverse-engineering
---
# ZGEOM Class Reversing

## Objective

Fully reverse one existing Glacier class derived from `ZGEOM`. The user supplies:

- the exact C++ class name;
- the PC vtable address;
- the path to the class `.h` file;
- the path to the class `.cpp` file.

The primary target is the `PC` Hyper/IDA instance and its 32-bit MSVC ABI. This workflow may edit the supplied source files and rename or comment proven functions and globals in the PC IDA database. It must first run the existing `vtable-annotating` skill for the class, using the supplied header, class, vtable address, instance `PC`, and ABI `MSVC`.

This skill applies only to `ZGEOM`-based entities. Before modifying anything, prove from the header inheritance chain that the class derives from `ZGEOM`. If it does not, stop and explain that this workflow is not applicable.

## Required Inputs

Resolve all four required inputs before analysis. Ask one focused question for any missing value; do not guess paths, class names, or addresses. Normalize addresses as hexadecimal and confirm that both source paths exist.

Inspect the active Hyper instances with `hyper_list_instances`. The `PC` instance is mandatory. Comparison instances are optional evidence; their absence does not justify inventing declarations or behavior.

Read before editing:

- the complete target header and implementation;
- every project base-class declaration needed to reconstruct inherited virtual methods;
- the relevant macro definitions in `Glacier/Runtime/Macro.h`;
- the RTP property layouts in `Glacier/RTP/Base.h`, property types in `Glacier/RTP/PropertyTypes.h`, and property table aliases in `Glacier/RTP/VirtualTables.h`;
- nearby completed ZGEOM classes with a similar base class or vtable shape.

Preserve current project style, namespace, includes, existing implementations, and unrelated user changes. Never change the class layout or vtable merely to make the reconstruction convenient.

## Evidence Priority

Use evidence in this order:

1. PC vtable targets, PC pseudocode, assembly, xrefs, globals, strings, allocation sites, and call sites.
2. The current project header, base-class headers, macros, and already verified neighboring implementations.
3. iOS, because it is generally the closest logical comparison to PC, while still allowing real differences.
4. PS2, especially for identifying virtual method behavior and original debug-era logic.
5. `XBOX_MiniNinjas` for additional type and naming evidence only; never assume its layout or behavior exactly matches PC.

Always validate a comparison-platform conclusion against PC. For `ZHM3*` types, use PS2 and iOS as the comparison sources; do not rely on `XBOX_MiniNinjas` or `XBOX_KL2`, where those types are absent.

When symbols are available through xexe, use `xexe_search_types`, `xexe_search_functions`, `xexe_search_globals`, `xexe_find_global_by_address`, and `xexe_find_globals_by_type` to corroborate types, signatures, source ownership, and names. Treat xexe results as comparison evidence, not as a substitute for PC verification.

If platforms conflict, preserve the PC behavior and layout. Record the exact conflicting slot/function/global addresses. Ask the user only if exhaustive PC and comparison analysis cannot resolve a declaration or source-changing decision safely.

## End-To-End Workflow

Perform the following phases in order. Do not stop after annotating the vtable or recovering RTTI; complete source edits and verification in the same run unless blocked by genuinely ambiguous evidence.

### 1. Establish The Baseline

1. Read the target class, all bases through `ZGEOM`, and the current `.cpp`.
2. Record the current class size assertion, members, constructors, declared virtual methods, existing RTTI macro data, and implemented methods. Do not discard already reversed work.
3. Verify that the supplied PC address is a plausible vtable by reading its pointer table and checking several targets against the class/base behavior.
4. Determine the complete physical vtable boundary. Do not infer it solely from existing names or comments.

### 2. Run Vtable Annotation

Load and execute the `vtable-annotating` skill with these fixed arguments:

```text
header: supplied .h path
class: supplied class name
vtable: supplied PC address
instance: PC
ABI: MSVC
```

Follow that skill's full workflow and comment every proven physical slot at the slot address. Keep its slot table as the working vtable map for all later phases.

For this Glacier workflow, logical slot `0` is the class `Release` entry. Decompile its PC target and follow the destruction path it invokes. Identify and rename the proven class destructor and `Release` function with readable class-qualified names through `hyper_rename_function`, using compare-and-set protection and never forcing over an unexpected meaningful name. Do not assume that the first physical MSVC entry is a destructor merely because generic C++ ABI documentation says so; use the Glacier logical layout and the PC body.

Resolve every other slot against the inheritance chain. Distinguish:

- inherited implementations that remain owned by a base class;
- target-class overrides occupying inherited slots;
- new virtual methods introduced by the target class;
- thunks or shared implementations that do not justify a new source declaration.

Rename only proven target-class PC functions. A shared base implementation must retain its base identity. Use `hyper_search_and_decompile` or per-instance searches/decompilation when comparing PC with iOS or PS2.

### 3. Recover Mandatory Per-Class Overrides

Every concrete class in this workflow is expected to provide class-specific overrides for:

```cpp
const RTP::ZPropertyInfo& GetProperties() const override;
uint32_t GetObjectId() const override;
void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
ZGEOMCLASSINFO* GetOldClassInfo() const override;
```

Confirm all four from their PC vtable slots and bodies rather than adding them mechanically.

- `GetProperties` returns the class property-info global.
- `GetObjectId` returns the class ID global.
- `GetObjectIdAndMask` reads or assigns the class ID global first and mask global second.
- `GetOldClassInfo` returns the class old-class-info global.

Rename each proven PC function to its class-qualified method identity if necessary. Rename the globals immediately after identification:

```text
old class-info global -> ClassName::m_OldClassInfo
property-info global  -> ClassName::Info
ID global             -> ClassName::m_Id
mask global           -> ClassName::m_Mask
```

Use `hyper_rename_global` with the observed name as `expected_name`. Never force-overwrite an unexpected meaningful name. Record each address exactly as it appears in the PC image.

### 4. Recover Class Registration And Type ID

Start from the global returned by `ClassName::GetOldClassInfo`:

1. Find all PC references to the global with `hyper_find_references`.
2. Normally there are two relevant references: the getter and a registration/initialization function. Verify this shape; do not select references by list position alone.
3. Decompile the non-getter reference and identify the initialization of the original `ClassName_Producer`/`ClassName__Producer` and its `ZGEOMCLASSINFO` payload.
4. Recover and record:

```text
TypeId
expected class size
base/parent class name
FactoryName string contents
FactoryName string address
```

5. Corroborate the parent against the C++ inheritance declaration and the size against PC allocation/layout evidence. A mismatch is a blocker to blind source editing and must be investigated.
6. Use the recovered `TypeId` in `DECLARE_GEOM_CLASS(ClassName, TypeId);`, preserving the project's hexadecimal and unsigned-suffix style.

The macro argument named `FactoryAddr` is the address of the factory-name string storage used by the producer, not the producer object or registration function address. Verify the bytes/string and references before recording it.

### 5. Recover RTTI Addresses

Record the following PC addresses for `DECLARE_GEOM_CLASS_IMPL`:

| Macro argument | Proven source |
| --- | --- |
| `OldClassInfoAddr` | global returned by `GetOldClassInfo` |
| `FactoryName` | exact string recovered from producer initialization |
| `FactoryAddr` | PC address of that factory-name string |
| `FirstProperty` | first proven node from the property-chain phase, otherwise `nullptr // TODO: Finish me` |
| `PropertiesAddr` | global returned by `GetProperties` |
| `IDAddr` | first class global used by `GetObjectIdAndMask`, corroborated by `GetObjectId` |
| `MaskAddr` | second class global used by `GetObjectIdAndMask` |

Do not substitute guessed addresses from another platform. Format PC image addresses consistently with neighboring source, normally as zero-padded `0x00......` values.

### 6. Decode The Property Chain

Attempt to reverse `ClassName::Info` as part of the normal workflow. A property chain is optional only when PC evidence is absent or genuinely ambiguous; do not leave `FirstProperty` as `nullptr` merely because the property globals lack useful names.

#### 6.1 Decode `ZPropertyInfo`

Read the complete PC `RTP::ZPropertyInfo` at `PropertiesAddr` using the 32-bit layout from `Glacier/RTP/Base.h`:

```text
+0x00 cNode*         First
+0x04 ZPropertyInfo* Super
+0x08 const char*    Name
```

Verify all three fields:

- `First` is null or points to a plausible property node in PC data;
- `Super` points to the immediate base class's `Info` global;
- `Name` resolves to the target factory/class name, either directly or after static initialization.

If a field is zero in the image, inspect relocations, references, and CRT/static initialization before concluding it is absent. Some builds initialize property names or `ZPropertyInfo` fields at startup rather than storing final pointers in the file image.

#### 6.2 Walk Every Node

Starting at `Info.First`, follow `RTP::cNode::m_Next` until null. Detect cycles and reject pointers outside plausible PC data. For every node, record:

```text
node address
next address
property name pointer and string
filter value
property virtual-table address
property kind and value type
stored PC member offset or getter/setter PMFs
extra enum-info address when applicable
```

The common 32-bit prefixes are:

```text
RTP::cNode
+0x00 cNode*      m_Next
+0x04 const char* m_Name
+0x08 uint32_t    m_Filter

RTP::ZDataProperty<T>
+0x0C table pointer
+0x10 encoded member offset

RTP::ZEnumProperty
+0x0C table pointer
+0x10 encoded member offset
+0x14 ZEnumInfo* m_Info

RTP::ZVirtualProperty<T>
+0x0C table pointer
+0x10 getter PMF function
+0x14 getter this-delta
+0x18 setter PMF function
+0x1C setter this-delta
```

Verify these offsets against the current project definitions and the PC ABI. Do not force a node into one of these shapes if its references or table functions disagree.

#### 6.3 Identify Property Kind And Type

Treat the property table's Load/Save function pointers as the authoritative PC type evidence:

1. Read the two function pointers from the node's table.
2. Resolve or decompile both functions in PC.
3. Determine whether they access a member offset, invoke getter/setter PMFs, or use enum metadata.
4. Recover `T` from a meaningful template symbol when available, or from the serializer operation and property-type enum used by the body.
5. Corroborate the result against aliases in `Glacier/RTP/VirtualTables.h`, such as `VirtualTable_DP__5` for `ZDataProperty<ZGEOMREF>` or `VirtualTable_DP__7` for `ZDataProperty<int>`.

Do not infer the type from field size alone. Several RTP types have identical four-byte storage but different serialization semantics, including `int`, `uint`, `ZREF`, `ZGEOMREF`, `ZCOLOR`, and string-backed handles. In particular, a class member declared as `ZREF` may intentionally use `ZDataProperty<ZGEOMREF>` so level-file serialization resolves an object name rather than exchanging a raw integer.

For `ZEnumProperty`, separately recover and reconstruct its `ZEnumInfo` and linked `ZEnumEntry` chain before claiming completion. Preserve the exact names, values, count/size field, and ordering proven by PC. Use another platform only to recover missing labels and verify every recovered value against the PC storage and serializer behavior.

#### 6.4 Recover Names And Filters

Read each `m_Name` string and `m_Filter` from PC. If the PC name pointer is zero before runtime initialization:

1. Find writes or relocations targeting the node's `m_Name` field.
2. Inspect nearby PC strings and the translation unit's static initializer.
3. Compare the same class property chain in iOS and PS2 for names and order.
4. Validate comparison names against PC node count, links, types, filters, offsets, and class fields.

Property-node ordinal names such as `NamespaceItem_268` can differ across platforms and are not semantic evidence. Preserve a proven PC/source ordinal when available; otherwise choose a local name consistent with nearby reconstructed files without claiming it is an original symbol.

#### 6.5 Resolve Member Offsets Correctly

PC property loaders calculate an address from the `ZSerializableBase` reference passed to RTP plus the encoded offset. That encoded value may differ from the complete-object member offset because the original MSVC inheritance model applies a base-subobject adjustment.

Never blindly copy the raw PC integer into source and never reject a property solely because it differs from `offsetof(ClassName, member)`.

For each data or enum property:

1. Decompile its PC loader and verify exactly which input pointer receives the encoded offset.
2. Compute the candidate complete-object field from the verified class layout and member accesses.
3. Measure the PC base-subobject delta using known class/base layout evidence or a nearby already reconstructed property using the same inheritance path.
4. Confirm `encoded PC offset + base adjustment == complete-object member offset`.
5. Corroborate with iOS or PS2 offsets when useful, accounting for their different ABI and base layouts.
6. Emit `.m_Offset = CLASS_PROPERTY(ClassName, member)` in project source so the current compiler computes the correct project representation.

If the RTP property type differs from the C++ member type but their representation and PC serializer semantics are proven compatible, keep the class member type unchanged and cast only the descriptor offset, for example:

```cpp
.m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ClassName, m_rTarget))
```

Do not change class layout merely to make the property template and member spelling identical.

#### 6.6 Reconstruct Source Nodes

Declare nodes inside the existing RTTI `namespace cProperties` in dependency order, normally tail first so each later node can initialize `m_Next` with the next property:

```cpp
static RTP::ZDataProperty<ZGEOMREF> NamespaceItem_Tail
{
    .m_Node = {
        .m_Next = nullptr,
        .m_Name = "m_rTarget",
        .m_Filter = ProvenFilter
    },
    .m_VirtualTable = ProvenDataTable,
    .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ClassName, m_rTarget))
};

static RTP::ZDataProperty<int> NamespaceItem_Head
{
    .m_Node = {
        .m_Next = NamespaceItem_Tail,
        .m_Name = "m_iFlags",
        .m_Filter = ProvenFilter
    },
    .m_VirtualTable = ProvenDataTable,
    .m_Offset = CLASS_PROPERTY(ClassName, m_iFlags)
};
```

Use the matching `ZVirtualProperty<T>`, `ZEnumProperty`, or `ZVirtualEnumProperty<T>` form when proven. Add `Glacier/RTP/VirtualTables.h` only when needed for table aliases. Set `DECLARE_GEOM_CLASS_IMPL`'s `FirstProperty` to the exact chain head, relying on the project's node conversion where established.

If any node's link, name, filter, kind, type, enum metadata, or field/PMF target remains unresolved, do not invent a partial chain that silently omits it. Keep `nullptr, // TODO: Finish me` unless the known prefix can be represented without changing runtime behavior, and report the exact unresolved node addresses and evidence.

Safely rename proven PC node globals to readable class-qualified identities with `hyper_rename_global` and compare-and-set protection. Never overwrite an unexpected meaningful property name.

### 7. Reconstruct The Header

Declare every proven target-class override and every proven new target-class virtual method. Use the exact signatures established from PC and corroborated comparison evidence. Include `const`, references, pointer types, return types, overloads, and access level. Do not add an override for a slot that still points to the inherited base implementation.

Organize the public section in inheritance/vtable order:

```cpp
class ClassName : public BaseClass
{
public:
    // RTTI
    DECLARE_GEOM_CLASS(ClassName, TypeId);

    // vtbl
    ~ClassName() override;

    // ZSerializable
    // target overrides declared by ZSerializable

    // RTP::cBase
    const RTP::ZPropertyInfo& GetProperties() const override;

    // ZGEOM
    uint32_t GetObjectId() const override;
    void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
    ZGEOMCLASSINFO* GetOldClassInfo() const override;
    // other ZGEOM overrides

    // IntermediateBase
    // overrides introduced by each base, one section per base in inheritance order

    // ClassName
    // new virtual methods introduced by this class

    // methods
    ClassName(const char* psName, ZBaseGeom* pBaseGeom);

    // members
    // proven fields only
};
RE_VERIFY_SIZE(ClassName, ClassSize);
```

Use the actual base classes and sections; do not emit empty section comments unless neighboring files consistently retain them for inheritance clarity. Keep non-virtual RTTI property methods in the project's existing `RTTI Methods` region when present.

Never change field offsets, add speculative members, or alter inheritance to satisfy a decompile. Preserve `RE_VERIFY_OFFSET` assertions. Set `ClassSize` only from verified PC allocation/layout evidence; retain an already verified correct assertion. If size is unknown, do not invent one or weaken an existing assertion.

### 8. Reconstruct The Implementation

Implement all newly declared overrides in the supplied `.cpp` using the smallest behavior that matches PC pseudocode and current project types. Preserve existing correct implementations. Put ordinary method implementations first inside the existing namespace, followed by the RTTI region, then any separately required global region according to neighboring file style.

The four mandatory accessors normally become:

```cpp
const RTP::ZPropertyInfo& ClassName::GetProperties() const
{
    return ClassName::Info;
}

uint32_t ClassName::GetObjectId() const
{
    return ClassName::m_Id;
}

void ClassName::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
{
    id = ClassName::m_Id;
    mask = ClassName::m_Mask;
}

ZGEOMCLASSINFO* ClassName::GetOldClassInfo() const
{
    return ClassName::m_OldClassInfo;
}
```

Use these bodies only after confirming the PC implementations. Implement the destructor according to proven ownership behavior; `= default` is acceptable only when it preserves the observed semantics through member/base destruction.

For unreversed dependencies, follow `AGENTS.md`: do not invent layouts or stubs. Leave `// TODO: Finish this place after {ClassName} will be reversed` at the exact blocked call site, with the expected decompiled call commented below when useful. Use `ZASSERT(false)` instead of debug breaks, structured control flow instead of `goto`, and `stlp::` containers for original Glacier STL data.

### 9. Add The RTTI Region

At the bottom of the namespace, after method implementations, add or update exactly one class RTTI region:

```cpp
#   pragma region " --- RTTI --- "
    namespace cProperties
    {
    }

    DECLARE_GEOM_CLASS_IMPL(
        ClassName,
        BaseClass,
        OldClassInfoAddr,
        "FactoryName",
        FactoryAddr,
        nullptr, // TODO: Finish me
        PropertiesAddr,
        IDAddr,
        MaskAddr
    );
#   pragma endregion
```

Use the exact recovered class, base, string, and PC addresses. `FactoryName` is a C++ string literal in this invocation. If properties have already been proven and implemented in `namespace cProperties`, preserve the verified first property instead of replacing it with `nullptr`. Otherwise use exactly `nullptr, // TODO: Finish me` for `FirstProperty`.

Do not duplicate an existing macro invocation or RTTI region. Update incomplete arguments in place. Add `#include <Glacier/Runtime/Macro.h>` or other includes only when the target does not already receive the required declaration transitively and the local include style requires it.

### 10. Handle Additional Globals

Place non-macro globals found during method reversing after the RTTI region in the file's established globals region, if one exists. Add only globals whose PC address, type, ownership, and use are proven. The four macro-backed RTTI/ID globals belong in `DECLARE_GEOM_CLASS_IMPL`; do not define duplicate storage manually.

## IDA Mutation Rules

- Perform all authoritative renames in the `PC` instance.
- Use readable C++ class-qualified identities, consistent with existing PC naming.
- Rename a function/global only after its role is proven by body and references.
- Supply `expected_name` for compare-and-set protection whenever an existing name was observed.
- If the current name is already correct, treat it as verified.
- Never use `force: true` to overwrite an unexpected meaningful user name.
- If one implementation is shared by several vtable slots/classes, do not assign a falsely exclusive derived-class name. Prefer the actual owner or retain the existing meaningful name and use slot comments for logical identities.
- Keep vtable comments on slot addresses as required by `vtable-annotating`; function comments are supplementary and do not replace slot comments.

## Source Editing Rules

- Make the smallest correct changes to the supplied `.h` and `.cpp`.
- Do not rewrite unrelated formatting or reorder already established members/methods without evidence.
- Do not change class layouts or vtables while reversing existing Glacier classes.
- Do not add compatibility wrappers, speculative overloads, fake types, or placeholder members.
- Preserve project naming and parameter conventions from neighboring Glacier classes.
- Use `apply_patch` for manual edits.
- Do not commit unless the user explicitly asks.

## Verification

Before finishing, verify all of the following:

1. The existing `vtable-annotating` skill completed for every proven PC slot and comments target slot addresses.
2. Logical slot `0` was analyzed as `Release`, its destructor path was followed, and proven PC names were applied safely.
3. Every header virtual declaration maps to a proven vtable slot, and every proven target override/new virtual has one declaration and implementation.
4. `GetProperties`, `GetObjectId`, `GetObjectIdAndMask`, and `GetOldClassInfo` use the four correctly named globals.
5. `TypeId`, class size, base class, factory string, and all six relevant addresses were recovered from PC evidence rather than copied from another platform.
6. The header contains one correct `DECLARE_GEOM_CLASS` and retains valid size/offset assertions.
7. `ZPropertyInfo::First`, `Super`, and `Name` were inspected, and every reachable property node was either fully decoded or reported with its exact unresolved address.
8. Every implemented property has a PC-proven chain link, name, filter, kind, serializer type/table, and field offset or PMF target; raw PC offsets were reconciled with the MSVC base-subobject adjustment before using `CLASS_PROPERTY`.
9. The implementation contains one correct `DECLARE_GEOM_CLASS_IMPL` in the RTTI region, with `FirstProperty` set to the proven chain head or marked `nullptr, // TODO: Finish me`.
10. No duplicate macro-backed globals or speculative layouts were introduced.
11. Includes, declarations, definitions, namespace qualification, and signatures are internally consistent.
12. Run the narrowest configured build or compile target covering the changed Glacier source. Fix source-caused failures. If no usable build is configured, perform static searches for duplicate/missing declarations and report the environment limitation.
13. Review the final diff and ensure unrelated existing changes were neither reverted nor folded into the implementation.

## Result

Return a concise summary containing:

- class, base, PC vtable address, slot count, and verified class size;
- recovered `TypeId`, `FactoryName`, `OldClassInfoAddr`, `FactoryAddr`, `PropertiesAddr`, `IDAddr`, and `MaskAddr`;
- PC functions/globals renamed or already correctly named;
- recovered property-chain nodes, names, filters, RTP kinds/types, PC encoded offsets, resolved class members/PMFs, and any safe property-global renames;
- source files changed and important overrides implemented;
- build/static verification result;
- unresolved methods, dependencies, property chain, or cross-platform discrepancies.

Do not claim full class completion while any vtable slot, mandatory RTTI value, declaration/definition, or source-caused verification failure remains unresolved.
