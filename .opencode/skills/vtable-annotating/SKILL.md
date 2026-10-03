---
name: vtable-annotating
description: Reconstruct a C++ vtable from a project header and compiler ABI, then annotate every discovered vtable slot in an IDA database through Hyper MCP comments.
compatibility: opencode with Hyper MCP IDA tools and the project's source/build tools available
metadata:
  project: ReHitman
  workflow: reverse-engineering
---
# Vtable Annotation

## Objective

Annotate the vtable belonging to one existing C++ class. The user supplies:

- the path to the `.h` file;
- the class name;
- the vtable address;
- the Hyper/IDA instance name (for example `PC`);
- the compiler ABI: `MSVC` or `Clang`.

If the ABI is not supplied, ask for it before doing analysis. Do not silently choose one: MSVC and Clang can differ in destructor entries, secondary bases, thunks, and slot ordering.

This workflow changes only IDA database comments. Do not change source layouts, vtables, declarations, or function names unless the user separately requests it.

## Input Validation

1. Resolve the header path and read the complete class declaration, its directly and indirectly inherited classes, access sections, virtual declarations, overrides, destructors, and compiler attributes. Search the repository for the class definition and related declarations if the supplied file is an include or forwarding header.
2. Confirm that the class name and requested vtable address belong to the selected build/instance. Use Hyper searches, references, memory reads, and pointer-table inspection as appropriate. Never assume that a PC address belongs to another IDA instance.
3. Determine pointer width from the instance/build. For a 32-bit PC database, slot `i` is `vtableAddress + i * 4`; for a 64-bit database it is `vtableAddress + i * 8`. Use the actual ABI/build width, not the host platform.
4. Inspect local project conventions and any existing comments/names around neighboring vtables. Preserve established spelling, namespace qualification, destructor notation, and thunk annotations when they are clear.

## Recover The Layout

Use a compiler-generated vtable dump as the primary layout oracle, preferring an already generated dump in the repository/build artifacts. If one is not available:

- For MSVC, use the repository's configured compiler/toolchain to obtain the class layout/vftable dump (commonly `/d1reportAllClassLayout`, or the project's existing equivalent). Do not invent a command that would rebuild the whole project if a targeted translation unit or existing artifact is enough.
- For Clang, use the configured Clang invocation with `-Xclang -fdump-vtable-layouts` on a minimal translation unit that includes the supplied header, preserving the project's defines, target, and include paths.

Before trusting a dump, verify that it is for the requested class, target architecture, and configuration. Compare its first slots with the header and with the function pointers at the requested address. A dump from the wrong compiler, platform, optimization configuration, or class definition is not evidence.

Reconstruct the complete primary vtable in slot order:

1. Include ABI-mandated destructor variants and deleting/deallocating destructor entries when the compiler dump shows them. Give them unambiguous comments such as `~ZSerializable` and `ZSerializable::`-qualified names according to the dump and project convention; do not collapse two physical slots into one comment.
2. Include inherited virtual methods in their physical order, then the derived overrides at their actual slots. A method inherited without an override keeps the declaring class in its qualified name unless existing project convention clearly uses the final class.
3. Include every direct/indirect base sub-vtable represented at this vtable address. For multiple inheritance, do not merge secondary vtables into the primary table. If the supplied address is a secondary vtable, identify its base/subobject and annotate that address's own slot sequence.
4. Account for ABI metadata or non-function entries only when the selected ABI places them in the vtable and the dump confirms them. Do not label RTTI/type-descriptor data as methods.
5. Follow thunks as physical entries. Comment the slot with the logical method identity and add a short suffix such as ` [this-adjusting thunk]` only when the compiler dump or disassembly proves it is a thunk.

The local vtable may be incomplete or unmarked at its tail. Continue until the compiler-derived class layout ends, the next known vtable/data region begins, or the pointer/function evidence becomes invalid. Do not stop merely because local comments or function names end. Conversely, never annotate speculative slots just to reach a convenient count. Base-class slots are expected to be fully represented even when derived-class information is incomplete.

## Resolve Names

Inspect the source, compiler dump, existing symbols, and targeted Hyper decompilation before naming each slot. Names must be readable logical C++ method identities, for example:

```text
~ZSerializable
ZSerializable::PreSave
ZSerializable::PostLoad
ZGROUP::GetGroupId
```

Use one name per physical slot. Keep overload parameters only when needed to distinguish otherwise identical names. Prefer the source declaration's spelling over guessed names. A pure virtual slot is still annotated with its declared method name. If a slot is an unresolved compiler-generated thunk, retain the best proven method identity and mark the thunk, rather than using `sub_...` as the comment.

If evidence conflicts, stop and ask one focused question after recording the exact conflicting addresses and evidence. Do not guess names or shift every later slot because of one uncertain entry.

## Apply Hyper Comments

For each resolved slot, call `hyper_set_comment` on the **slot address**, not on the function target:

```text
slotAddress = vtableAddress + slotIndex * pointerSize
comment = resolvedName
```

Thus a 32-bit vtable at `0076A4C4` receives comments at `0076A4C4`, `0076A4C8`, `0076A4CC`, and so on. The function pointer stored in a slot is used for verification/decompilation but is not the comment address. Use the requested IDA instance for every call. Set regular comments unless the user explicitly requests repeatable comments.

Process slots in ascending order, one Hyper operation per slot. Do not batch several names into one comment. Use compare-and-set protection when an existing expected comment is known; never force-overwrite an unexpected user comment. If a slot already has the exact requested comment, treat it as verified. If Hyper reports a conflict, preserve the existing comment and ask the user rather than forcing it.

After every write, verify the slot pointer with `hyper_read_memory` or `hyper_read_pointer_table` and record the slot address, target address, and applied comment in the current working result. Hyper has no general comment-read operation in this tool set, so do not claim independent comment readback unless the tool response explicitly provides it.

## Verification

Before finishing, verify:

- the header class and ABI match the selected instance;
- slot addresses are contiguous using the correct pointer width;
- every compiler-derived physical slot has exactly one attempted/applicable Hyper comment;
- inherited/base methods are not skipped;
- destructor variants, thunks, overloads, and secondary-vtable boundaries are handled explicitly;
- no speculative tail slots were annotated;
- comments were written in the requested instance and to slot addresses, not target function addresses.

Return a concise table containing slot index, slot address, function target when known, and comment. Include the ABI, pointer width, number of slots, and any unresolved boundary or naming issue. Do not edit or commit project files as part of this skill.

## Focused Questions

Ask only for missing or genuinely ambiguous information. The minimum initial question must request the missing ABI if needed. If the path, class, address, or instance is missing, request those values too. If a compiler dump cannot be produced because the build configuration/toolchain is unavailable, ask for the relevant dump text or file path instead of fabricating a layout.
