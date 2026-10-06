# ZGEOM Class Reversing

This skill performs a complete reverse-engineering pass for one Glacier class derived from `ZGEOM`. The PC IDA database is the authoritative source; iOS, PS2, and available Xbox builds are used only to compare behavior and refine method signatures.

The skill automatically:

- verifies that the target class derives from `ZGEOM`;
- invokes the nested `vtable-annotating` skill and annotates the complete proven PC vtable;
- analyzes logical slot zero as `Release` and follows it to the invoked destructor;
- distinguishes inherited implementations, overrides, and virtual methods introduced by the target class;
- renames proven functions and globals in the PC IDA database;
- recovers the class `TypeId`, size, parent, and factory metadata;
- finds the addresses of `m_OldClassInfo`, `Info`, `m_Id`, and `m_Mask`;
- walks `Info.First` and decodes PC data, virtual, enum, and virtual-enum property nodes;
- identifies property types from their PC Load/Save tables rather than storage size alone;
- reconciles raw PC property offsets with the MSVC base-subobject adjustment before emitting `CLASS_PROPERTY`;
- updates the supplied `.h` and `.cpp` files;
- adds or updates `DECLARE_GEOM_CLASS` and `DECLARE_GEOM_CLASS_IMPL`;
- implements proven overrides without speculatively changing the class layout;
- runs the narrowest available build or static consistency check.

## Required Inputs

Provide all of the following values:

- the exact C++ class name;
- the address of its vtable in the PC build;
- the path to the class header (`.h`);
- the path to the class implementation (`.cpp`).

The skill automatically uses the `PC` Hyper/IDA instance and the `MSVC` ABI. If a required value is missing, the agent asks only for the missing information.

## Usage Example

```text
/reverse-zgeom-class class ZBackdrop, vtable 0x0076A4C4, header ReHitman/Glacier/include/Glacier/Geom/ZBackdrop.h, cpp ReHitman/Glacier/source/Geom/ZBackdrop.cpp
```

A natural-language request works as well:

```text
Fully reverse ZBackdrop as a ZGEOM-derived class. Its PC vtable is at 0x0076A4C4. Use ReHitman/Glacier/include/Glacier/Geom/ZBackdrop.h and ReHitman/Glacier/source/Geom/ZBackdrop.cpp.
```

## Analysis Sources

- `PC` is mandatory and authoritative for addresses, behavior, layout, and symbol renaming.
- iOS is generally the closest logical comparison to PC.
- PS2 helps recover virtual method behavior and original debug-build logic.
- `XBOX_MiniNinjas` may provide supplementary type and naming evidence, but it is not assumed to match PC exactly.
- For `ZHM3*` classes, comparisons use iOS and PS2; Xbox instances are not used because those types are absent there.

Every conclusion drawn from another platform must be validated against PC. RTTI macro addresses are never copied from another build.

## Property Recovery

The workflow treats `ClassName::Info` as a linked runtime schema. It reads the PC `ZPropertyInfo`, follows every `cNode::m_Next`, and records each node's name, filter, property table, encoded offset or member-function pointers, and enum metadata.

Property kind and type come from the authoritative PC Load/Save function pair. This distinguishes same-sized representations with different serialization behavior, such as a raw integer reference and `ZGEOMREF`. Missing runtime-initialized names may be recovered from PC initialization and corroborated with iOS or PS2, but the PC chain shape, type tables, filters, and offsets must still agree.

Raw PC offsets are not copied directly into C++. The original loader addresses fields relative to a `ZSerializableBase` subobject, so MSVC may encode an offset smaller than the complete-object member offset. The workflow proves that adjustment and emits `CLASS_PROPERTY(ClassName, member)`, preserving the actual class layout. If the property serializer type differs from the member spelling but has a proven compatible representation, only the descriptor offset is cast.

## Expected Result

After a successful run:

- every proven PC vtable slot has a separate IDA comment;
- `Release`, the destructor, and target-class methods have proven class-qualified names;
- globals are named `ClassName::m_OldClassInfo`, `ClassName::Info`, `ClassName::m_Id`, and `ClassName::m_Mask`;
- the header contains the correct `DECLARE_GEOM_CLASS(ClassName, TypeId)` declaration;
- overrides are grouped by owner, including `ZSerializable`, `RTP::cBase`, `ZGEOM`, intermediate bases, and the target class;
- existing `RE_VERIFY_SIZE` and `RE_VERIFY_OFFSET` assertions are preserved unless PC evidence proves another value;
- the implementation contains the smallest method bodies matching the PC decompilation;
- the bottom of the namespace contains one updated RTTI region with `DECLARE_GEOM_CLASS_IMPL`;
- a recovered property chain is reconstructed in `namespace cProperties` and supplied as `FirstProperty`;
- an ambiguous property chain remains `nullptr, // TODO: Finish me`, with exact unresolved PC nodes reported instead of guessed;
- the relevant build target or a static consistency check has completed.

The skill must not invent members, alter inheritance, or speculatively extend the vtable. If a method depends on an unreversed class, it uses the TODO convention from the repository-level `AGENTS.md`.

## Related Files

- Main workflow: `.opencode/skills/zgeom-class-reversing/SKILL.md`
- Slash command: `.opencode/commands/reverse-zgeom-class.md`
- Nested vtable workflow: `.opencode/skills/vtable-annotating/SKILL.md`
