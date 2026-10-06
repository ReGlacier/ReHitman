# TexCapture (nvCPL.dll drop-in)

Micro-tool, same auto-load trick as `ReHitman/NVCPL`: Hitman Blood Money
delay-loads `nvCPL.dll` and calls the exported `NvCplGetDataInt` at startup, so
simply putting this DLL next to `HitmanBloodMoney.exe` makes it run inside the
game - no ReHitman.dll or injection framework needed.

## How it works

Mirrors the proven ReHitman/BloodMoney D3D hook (`ZDirect3D9DevicePatches`):
- on load, `nvCPL.dll` NOPs the game's `FreeLibrary` call (0x004884D9) so the
  dll stays resident;
- a small setup thread only waits until the renderer global `g_pd3dDevice` is
  set, then swaps the D3D device's **Present** and **GetSwapChain** vtable slots
  (no D3D calls from the helper thread!);
- when **F5** is pressed, the capture runs inside the Present callback - i.e.
  on the render thread, exactly where ReHitman draws its overlay.

The output files (`<game dir>\ref_data\`):

- `mapCubeNormalizer_face0..5.raw` (128x128 cube faces)
- `mapAnisotropic.raw`, `mapSpecular.raw`, `mapScatter.raw`,
  `mapHgPhase.raw`, `mapRefraction.raw` (128x128)
- `spotAttenMap0_r0.raw / _r1.raw` and `spotAttenMap1_r0.raw / _r1.raw` (32x32)
- `texFont8x13.raw` (128x128), `texMouse16x16.raw` (16x16),
  `texShadowClip1x2.raw`, `texWhite1x1.raw`
- `info.txt` with `name width height` lines

The outputs are used as reference data for the CPU-side generators in
`ZSharedResourcesD3D.cpp` (compare with the engine reimplementation; the test
suite under `ReHitman/Glacier/Tests` runs on bundled test data).

The global addresses are hard-coded from the PC build (no ASLR); the module
base is resolved at capture time.

## Build

```
cmake -S Tools/TextureCapture -B build/texcap -A Win32
cmake --build build/texcap --config Release
```
Copy `build/texcap/Release/nvCPL.dll` into the game folder (rename/back up any
existing `nvCPL.dll` - do not run it together with the ReHitman NVCPL injector).

## Notes / caveats

- The device `Present` vtable hook fires every frame on the render thread; the
  capture itself only happens when F5 is pressed (cooldown 1.5 s), in the menu
  or pause preferably.
- Textures that need `D3DPOOL_DEFAULT` render targets (shadow colour maps, env
  cubemaps) are intentionally not dumped here.
- Keep only one `nvCPL.dll` next to the exe (do not run it together with the
  ReHitman NVCPL injector).
