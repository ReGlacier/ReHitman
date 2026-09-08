# PF4 NavMesh Viewer

Standalone HTML5 viewer for the **PF4 (pathfinder4)** navmesh chunk stored inside
Hitman: Blood Money `.GMS` level files.

## Quick start (no web server needed)

Just double-click **`index.html`** and open a `.GMS` file (or drag & drop it onto
the page). All libraries (`three.min.js`, `OrbitControls.js`, `pako.min.js`) are
vendored locally, so the page works from `file://` and offline.

## Or serve it via npm

```bash
npm start      # uses `python -m http.server 8000`
npm run serve  # uses `npx http-server` (auto-downloads on first run)
```

then open http://localhost:8000

## What it does

1. Reads the GMS container the same way the reference dumper does:
   - header `i32 uncompressedSize, i32 bufferSize, i8 notCompressed`
   - the payload is raw-deflate (`wbits=-15`) unless already uncompressed
   - `u32` at payload offset `0x34` points to the PF4 chunk (`u32 size` + data)
2. Parses the PF4 block using the layout reversed from the PC binary
   (`SPF4DataBlock` header + packed arrays):
   `splitTrees, heightTrees, planeEquations, components, graphs, nodes,
    vertices, links, exitDists, corners, subNodes, staticObstacleIds,
    staticObstacles`
3. Renders every walkable component polygon in 3D (vertex heights included),
   colored per graph, with wireframes, orbit camera and click-to-inspect.

## UI

- **Open .GMS** — file picker (also supports drag & drop anywhere on the page)
- **Frame** — refit the camera to the mesh
- **Wireframe** — toggle cell outlines
- **components / links** — show/hide the component cells and the link nodes
- Bottom-left panel shows the array counts of the loaded chunk
- Click a cell to inspect its graph summary

## Notes

- Only the navigation data is shown; no level geometry/assets are used.
- The `.GMS` file must contain a PF4 chunk with a **normal** header (the
  `0xFFFFFFFC` chained-chunk case is reported as an error).
- Local copies of the viewer libraries:
  `three.js r128` (UMD), its `OrbitControls`, and `pako` inflate.

## Reference

- Dump format key used by the game: `ReHitmanT00l$2k26` (see
  `ReHitman/Glacier/Tests/data/` for an obfuscated sample).
- Data layout: `ZData::LoadDataBlock` in
  `ReHitman/Glacier/source/PF4/ZData.cpp`.
