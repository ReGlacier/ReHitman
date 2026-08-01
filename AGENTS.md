# Agent Notes

- Do not change class layouts or vtables while reversing existing Glacier classes.
- Prefer the smallest correct implementation that matches the PC decompile and current project types.
- If decompiled code references an entity that is not reversed yet, do not invent stubs or new layout. Leave a TODO at the call site.
- TODO format: `// TODO: Finish this place after {ClassName} will be reversed`.
- When useful, include the expected decompiled call as commented example code below the TODO.
- Do not hardcode source file string literals or line numbers for replay/debug call-site arguments. Use `__FILE__` and `__LINE__`.
- Use `ZASSERT(false)` instead of `__debugbreak()` or `DebugBreak()` in project code.
- Do not edit vendored third-party code for these conventions unless explicitly requested.
