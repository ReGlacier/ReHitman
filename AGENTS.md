# Agent Notes

- Do not change class layouts or vtables while reversing existing Glacier classes.
- Prefer the smallest correct implementation that matches the PC decompile and current project types.
- If decompiled code references an entity that is not reversed yet, do not invent stubs or new layout. Leave a TODO at the call site.
- TODO format: `// TODO: Finish this place after {ClassName} will be reversed`.
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
