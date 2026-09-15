import struct
import os

base = r"Tools/ida_dump/result"
font = open(os.path.join(base, "procfont.bin"), "rb").read()
mouse = open(os.path.join(base, "promouse.bin"), "rb").read()
assert len(font) == 1235, len(font)
assert len(mouse) == 1024, len(mouse)
dwords = struct.unpack("<256I", mouse)


def fmt(data, per):
    lines = []
    for i in range(0, len(data), per):
        lines.append(", ".join(str(v) for v in data[i : i + per]))
    return ",\n    ".join(lines)


out = []
out.append(
    "// Auto-generated engine asset data used by ZSharedResourcesD3D::Allocate:"
)
out.append(
    "// 8x13 glyphs (ASCII 0x20..0x7E) and the 16x16 mouse cursor."
)
out.append("// Re-generated with Tools/ida_dump/gen_procedural_inc.py.")
out.append("")
out.append("static const uint8_t kProceduralFontData[1235] = {")
out.append("    " + fmt(font, 20))
out.append("};")
out.append("")
out.append("static const uint32_t kProceduralMouseData[256] = {")
out.append("    " + fmt(dwords, 8))
out.append("};")
out.append("")
open(
    r"ReHitman/Glacier/source/Render/ZSharedResourcesD3D_ProceduralData.inc", "w"
).write("\n".join(out))
print("written", len("\n".join(out)))
