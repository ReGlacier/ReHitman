# IDA disassembler exporter for the render map generators

`export_render_disasm.py` prints an exact scalar x86 disassembly (with decoded
float constants and resolved data / call references) for the renderer's "map
generator" functions:

`CreateScatterMap`, `CreateSpotAttenuationMap`, `CreateAnisotropicMap`,
`CreateSpecularMap`, `CreatehgPhaseMap`, `CreateRefractionMap`,
`CreateNormalizerMap`, their leaf helpers `sub_48C0F0` / `sub_48D7E0` /
`sub_48DCE0` / `sub_48DEB0` / `sub_48E150` / `sub_48E0E0`, plus
`ZSharedResourcesD3D::Allocate`.

## Why

The Hex-Rays pseudocode of these routines is mangled by `__usercall`
conventions and x87 / NEON leftovers, so a line-by-line scalar disassembly with
decoded constants is needed to reconstruct the pixel math precisely.

## Usage

1. Open the database in IDA.
2. `File → Script file…` → pick `export_render_disasm.py` (or run `ida -S`).
3. Outputs are written next to the database:
   - `<db>_render_disasm.txt` — instruction listing with data/call resolution
     and float constants,
   - `procfont.bin` — 1235 bytes of 8x13 glyph data (ASCII 0x20..0x7E),
   - `promouse.bin` — 1024 bytes of 16x16 cursor pixels.

Each instruction line is annotated with:
- `data[0x…: u32=… i32=… f32=…]` — for .data/.rdata references (4 bytes),
- `data[0x…: 4xf32=[…]]` — for xmmword references (4 floats),
- `data[0x…: f64=…]` — for qword references (double),
- `imm:u32=… f32=…` — for immediate operands,
- `-> funcname` — for call/jmp targets that resolve to a function.

## Configuration

The set of root functions lives in `PC_FUNCTIONS` at the top of the file
(by name or address). Callees are collected automatically (breadth-first,
bounded by `MAX_FUNCTIONS`), so leaf helpers reached from the roots are
included even if they are not listed explicitly. Addresses are only a
fallback: functions already named in the database (e.g. `CreateScatterMap`)
are resolved by name.

## Companion scripts

- `gen_procedural_inc.py` — turns the exported glyph / cursor data into the
  C++ include used by the renderer.
- `dump_process_mem.py` — reads selected renderer globals from a running build
  (module base auto-resolved), for comparison with the engine reimplementation.
- `verify_dump.py`, `verify_textures.py` — reference checkers that compare the
  reimplementation's CPU-side output against the reference data.

The reference outputs consumed by these scripts and by the test suite are kept
out of this repository; the tests under `ReHitman/Glacier/Tests` use bundled
test data and skip when it is not present.
