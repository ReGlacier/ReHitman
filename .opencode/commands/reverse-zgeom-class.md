---
description: Reverse one PC ZGEOM-derived class from its vtable and source files
subtask: false
---
Load and run the `zgeom-class-reversing` skill in the current context. Obtain these required values from the user before analysis if any are missing:

- exact C++ class name;
- PC vtable address;
- class header path;
- class implementation `.cpp` path.

Use the `PC` Hyper/IDA instance and MSVC ABI as the authoritative target. First execute the existing `vtable-annotating` skill for the supplied class and vtable, then analyze logical slot 0 as Glacier `Release`, follow and name its destructor path, recover every target-class override, and safely rename proven PC functions and globals with compare-and-set protection. Recover `TypeId`, class size, base class, factory string/address, property-info address, old-class-info address, ID address, and mask address from PC pseudocode and references. Use iOS and PS2 only as comparison evidence and always verify conclusions against PC; for `ZHM3*` types, use iOS and PS2 rather than Xbox instances.

Update the supplied header with the verified `DECLARE_GEOM_CLASS`, complete override declarations grouped by inheritance owner, and preserved layout assertions. Update the implementation with the smallest PC-matching method bodies and one `DECLARE_GEOM_CLASS_IMPL` RTTI region; use `nullptr, // TODO: Finish me` for the first property unless the property chain is already proven. Follow `AGENTS.md`, never invent unreversed layouts or dependencies, never alter the class layout/vtable speculatively, and never overwrite unexpected meaningful IDA names. Run the narrowest applicable verification and return the recovered metadata, IDA mutations, source changes, and unresolved evidence only after the end-to-end workflow is complete.
