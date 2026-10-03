# Agent Notes

- Do not change class layouts or vtables while reversing existing Glacier classes.
- Prefer the smallest correct implementation that matches the PC decompile and current project types.
- **Check before leaving TODOs:** Before marking a class or entity as missing, search the repository to confirm whether it is already reversed or available under another header.
- **Handling unreversed entities:**
  - If a required entity is completely missing from the codebase, search the local codebase first, then leave a TODO at the call site rather than inventing a layout or stub.
  - **TODO format:** `// TODO: Finish me` with an optional reason, for example `// TODO: Finish me after ZMissingClass reversed`.
  - If the entity already exists, call its available methods or fields. If a member is missing, update that class first where possible; otherwise use `// TODO: Finish me after ClassName::MemberName reversed`.
- When useful, include the expected decompiled call as commented example code below the TODO.
- Do not hardcode source file string literals or line numbers for replay/debug call-site arguments. Use `__FILE__` and `__LINE__`.
- Use `ZASSERT(false)` instead of `__debugbreak()` or `DebugBreak()` in project code.
- In PS2 decompiled code, `sCall_u::_uPrint` means debug output. Replace it with `printf` and include a trailing newline.
- If `_break(0, 0)` follows debug output, replace the break with `ZASSERT(false)`.
- When the debug output clearly represents an assert, replace the whole pattern with the equivalent `ZASSERT(...)` while preserving the original condition as closely as possible. For example, a decompiled block that prints `ZASSERT(p->GetType() == ZComponentProducerData::CHILD)` and then calls `_break(0, 0)` should become `ZASSERT(p->GetType() == ZComponentProducerData::CHILD);`.
- Do not edit vendored third-party code for these conventions unless explicitly requested.
- In Glacier code, do not use the host C++ Standard Library containers or strings for reversed engine data. The original game used STLport 4.6.1; when decompiled code mentions `std::map`, `std::vector`, `std::string`, `std::set`, or similar STL entities, use the STLport target and write them as `stlp::map`, `stlp::vector`, `stlp::string`, `stlp::set`, etc.
- Connect STLport through the CMake target named `stlport`. Do not add ad-hoc include paths to `STLport-4.6.1/stlport`; link the target instead so the configured `stlp` namespace and compatibility defines are applied.
- Prefer `stlp::` explicitly in Glacier headers and sources for original STL containers. Do not alias `stlp` to `std`, and do not use host `std::` containers in binary-compatible Glacier layouts.
- Never use ordinary `new`, `new[]`, `delete`, or `delete[]` in project code. Use `ZUniMemory::Allocate`/`ZUniMemory::Free` instead; placement-new construction through the project's placement-new mechanism is the only exception.
- Never use `goto` in project code. Rewrite decompiled control flow that relies on `goto` with structured constructs (loops, `break`/`continue`, early `return`, or small helpers) while preserving the original semantics.
- **Choosing a data source (MCP tools):**
  - **MCP xexe:** Data source strictly for the PS2 build. Do NOT query it for anything render- or DirectX (d3d)-specific. Every access to xexe must be approved by the user first — ask before using it.
  - **MCP hyper:** Use for all decompilation, reversing, and type-database questions. Follow this platform priority order:
    1. **PC** – highest priority for code, references, vtbl, and layout.
    2. **XBOX_KL1** – closest to PC; use when PC info is missing or insufficient.
    3. **iOS** – close to PC but with its own modifications; use when XBOX_KL1 does not match. Contains function and code info only (no type info).
    4. **XBOX_MiniNinjas** – next closest.
    5. **XBOX_KL2** – furthest overall, but may have more debug symbols than XBOX_MiniNinjas.
    6. **PS2** – furthest overall, but one of the closest by game code and layout; carries info specific to Hitman Blood Money and early Glacier engine roots (excluding render and sound).
- At the end of your work, you MUST verify that the code compiles. Follow these build rules strictly:
  - **Never invent or guess build commands.** This project is built exclusively with CMake. Use only the documented CMake workflow below.
  - **Always build in the `build` directory.** It already exists and is configured; do not reconfigure it or build in any other directory unless the user explicitly asks.
  - **Regenerate when new `.cpp` files are added.** If the session created new C++ source files, CMake must regenerate the build system before building, otherwise the new files will not be compiled.
  - **Steps:**
    1. Regenerate (only if new `.cpp` files were added this session): `cmake -B build -G "Visual Studio 17 2022" -A Win32`
    2. Build: `cmake --build build --config Debug -j`
  - If a build command is missing or unclear, ask the user for it rather than guessing.
