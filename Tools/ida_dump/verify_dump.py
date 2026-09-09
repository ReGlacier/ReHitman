import struct

def rd_u32(b, off):
    return struct.unpack_from("<I", b, off)[0]

def rd_f(b, off):
    return struct.unpack_from("<f", b, off)[0]

data = open(r"ReHitman/Glacier/Tests/data/font_letter_positions.bin", "rb").read()
assert len(data) == 0x400

bad = 0
for c in range(0x20, 0x7F):
    off = c * 8
    u = rd_f(data, off)
    v = rd_f(data, off + 4)
    eu = float(8 * (c & 0xF)) * (1.0 / 128.0)
    ev = float(13 * (c // 16)) * (1.0 / 128.0)
    if u != eu or v != ev:
        bad += 1
        if bad < 10:
            print("MISMATCH char=%02X u=%r v=%r expected %r %r" % (c, u, v, eu, ev))
print("checked chars 0x20..0x7E; mismatches =", bad)

# rows: dump = u v pairs every 8 bytes
print("sample ' '(0x20):", rd_f(data, 0x20*8), rd_f(data, 0x20*8+4))
print("sample 'A'(0x41):", rd_f(data, 0x41*8), rd_f(data, 0x41*8+4))

# spot slot handles (first dword of each 0x44 element)
for name, n in (("g_pSpotAttenuationMap.bin", 2), ("g_pSpotAttenuationMap2.bin", 2)):
    b = open("ReHitman/Glacier/Tests/data/" + name, "rb").read()
    print(name, "handles:", [hex(rd_u32(b, i * 0x44)) for i in range(n)])

# wrapper userData pointers & name strings
for fname, nm in (
    ("g_texWhite.bin", "White1x1"),
    ("g_texMouse16x16.bin", "Mouse16x16"),
    ("g_texFont8x13.bin", "Font8x13"),
    ("g_texShadowClip1x2.bin", "ShadowClip1x2"),
):
    b = open("ReHitman/Glacier/Tests/data/" + fname, "rb").read()
    ud = rd_u32(b, 0x2C)
    nm_raw = b[0x14:0x2C].split(b"\0")[0].decode("latin1")
    print(fname, "userData=", hex(ud), "name=", repr(nm_raw), "expected=", nm)
