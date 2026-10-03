---
description: Annotate a C++ vtable in an IDA database using the class header and compiler ABI
subtask: false
---
Run the `vtable-annotating` skill in the current context. Obtain these required values from the user before analysis if they are not all present:

- header path (`.h` file);
- exact C++ class name;
- vtable address;
- Hyper/IDA instance name;
- compiler ABI: `MSVC` or `Clang`.

If the user omitted the ABI, ask specifically whether the target was built with MSVC or Clang. Inspect the source class and all base classes, obtain or generate the matching compiler vtable dump, recover the physical slot order, and annotate each slot separately with `hyper_set_comment` in the supplied instance. Comments must be placed at `vtable address + slot index * pointer size`; use the stored function pointer only to validate and identify the slot. Include inherited methods and ABI destructor/thunk entries, continue through the compiler-derived vtable even if local IDA marking stops early, and stop at a proven table boundary rather than guessing. Never overwrite an unexpected existing comment. Return the final slot mapping and any unresolved ambiguity only after all proven slots have been processed.
