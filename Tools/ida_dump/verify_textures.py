import math
import os
import struct

D = r"ReHitman/Glacier/Tests/data"


def rd(path):
    return open(path, "rb").read()


def u32(v):
    return struct.pack("<I", v & 0xFFFFFFFF)


def round_byte(v):
    i = int(v + 0.5)
    if i > 255:
        return 255
    if i < 0:
        return 0
    return i


def trunc_byte(v):
    i = int(v)
    if i > 255:
        return 255
    if i < 0:
        return 0
    return i


def pack(r, g, b):
    return 0xFF000000 | (r << 16) | (g << 8) | b


def clamp01(v):
    return 0.0 if v < 0.0 else (1.0 if v > 1.0 else v)


def scatter(x, y, w, h):
    u = 2.0 * (x + 0.5) / w - 1.0
    v = 2.0 * (y + 0.5) / h - 1.0
    if u == 0.0 or v == 0.0:
        return 0xFF000000
    au, av = abs(u), abs(v)
    f = 1.0 - math.exp(-(1.0 / au + 1.0 / av) * 0.5)
    d = au + av
    return pack(round_byte(f * 0.8 / d * 63.75),
                round_byte(f * 0.5 / d * 63.75),
                round_byte(f * 0.4 / d * 63.75))


def specular(x, y, w, h):
    angle = (y + 0.5) / h * 1.5707964
    power = (x + 0.5) / w * 128.0
    c = round_byte(math.pow(math.cos(angle), power) * 255.0)
    return c | c << 8 | c << 16 | c << 24


def aniso(x, y, w, h):
    col = 2.0 * (x + 0.5) / w - 1.0
    row = 2.0 * (y + 0.5) / h - 1.0
    dot = col * row + math.sqrt(1 - col * col) * math.sqrt(1 - row * row)
    a = round_byte(math.pow(dot, 128.0) * 255.0)
    c = round_byte(math.sqrt(1 - col * col) * 255.0)
    return (a << 24) | (c << 16) | (c << 8) | c


def hg(x, y, w, h):
    u = 2.0 * (y + 0.5) / h - 1.0

    def fn(g):
        num = 1 - g * g
        den = g * g + 1 - 2 * g * u
        return num / math.pow(den, 1.5)

    return pack(trunc_byte(fn(0.8) * 63.75),
                trunc_byte(fn(0.3) * 63.75),
                trunc_byte(fn(0.0) * 63.75))


def refraction(x, y, w, h):
    fu = 2.0 * (x + 0.5) / w - 1.0
    if fu == 0.0:
        return 0
    kk, kc = 0.5102041059610798, 0.7142857313156128
    fneg = -fu
    ft = 1.0 - (1.0 - fu * fu) * kk
    fs = 0.0 if ft < 0.0 else 1.0
    frt = math.sqrt(ft) if ft > 0.0 else 0.0
    fc = kc * fs
    fm = (fneg * kc - frt) * fs
    fd = -(fc * fu + fm)
    fe = fneg / fd if fd != 0.0 else 0.0
    fp = fd / fneg if fneg != 0.0 else 0.0
    fa = (fe - kc) / (fe + kc)
    fb = (fp - kc) / (fp + kc)
    fh = (fa * fa + fb * fb) * 0.5 * fs + (1.0 - fs)
    fr = clamp01(fc)
    fg = clamp01(-fm)
    s = clamp01(fh)
    if s > 0.5:
        s = 0.5
    t = s / 0.5
    sm = 3 * t * t - 2 * t * t * t
    la = int((1.0 - sm) * -255.0)
    lx = int(fr * -255.0)
    lz = int(fg * -255.0)
    res = (0xFF00 - lx) << 8
    res = ((res - lz) << 8) - la
    return res & 0xFFFFFFFF


def normalizer(face, x, y, w, h):
    col = 2.0 * (x + 0.5) / w - 1.0
    row = 2.0 * (y + 0.5) / h - 1.0
    bases = [
        (1.0, lambda: -row, lambda: -col),
        (-1.0, lambda: -row, lambda: col),
        (lambda: col, 1.0, lambda: row),
        (lambda: col, -1.0, lambda: -row),
        (lambda: col, lambda: -row, 1.0),
        (lambda: -col, lambda: -row, -1.0),
    ]
    comps = [v() if callable(v) else v for v in bases[face]]
    ln = math.sqrt(sum(c * c for c in comps))
    return pack(round_byte((comps[0] / ln * 0.5 + 0.5) * 255.0),
                round_byte((comps[1] / ln * 0.5 + 0.5) * 255.0),
                round_byte((comps[2] / ln * 0.5 + 0.5) * 255.0))


def spot(ratio, mode, x, y, w, h):
    col = 2.0 * (x + 0.5) / w - 1.0
    row = 2.0 * (y + 0.5) / h - 1.0
    v = 0.0
    if mode != 0:
        fx = clamp01((abs(col) - ratio) / (1.0 - ratio))
        fy = clamp01((abs(row) - ratio) / (1.0 - ratio))
        base = (1.0 - fx) * (1.0 - fy)
        v = base * base * (3.0 - 2.0 * base)
    else:
        dist = math.sqrt(col * col + row * row)
        if dist < 1.0:
            if dist <= ratio:
                v = 1.0
            else:
                t = 1.0 - (dist - ratio) / (1.0 - ratio)
                v = t * t * (3.0 - 2.0 * t)
    if y == 0 or y == h - 1 or x == 0 or x == w - 1:
        v = 0.0
    c = round_byte(v * 255.0)
    return c | c << 8 | c << 16 | c << 24


def render(w, h, fn):
    out = bytearray()
    for y in range(h):
        for x in range(w):
            out += u32(fn(x, y, w, h))
    return bytes(out)


def compare(name, w, h, fn):
    dump = rd(os.path.join(D, "ref_data", name))
    if len(dump) != w * h * 4:
        print("MISSING/size %s %d vs %d" % (name, len(dump), w * h * 4))
        return
    cpu = render(w, h, fn)
    diff = sum(1 for a, b in zip(cpu, dump) if a != b)
    print("%-32s diff bytes %d / %d  (%.3f%%)" % (name, diff, len(dump), 100.0 * diff / len(dump)))
    if diff:
        for i in range(len(cpu)):
            if cpu[i] != dump[i]:
                print("   first diff at byte %d: cpu=%02X dump=%02X (pixel %d)" % (i, cpu[i], dump[i], i // 4))
                break


compare("mapScatter.raw", 128, 128, scatter)
compare("mapSpecular.raw", 128, 128, specular)
compare("mapAnisotropic.raw", 128, 128, aniso)
compare("mapHgPhase.raw", 128, 128, hg)
compare("mapRefraction.raw", 128, 128, refraction)

for f in range(6):
    compare("mapCubeNormalizer_face%d.raw" % f, 128, 128,
            lambda x, y, w, h, f=f: normalizer(f, x, y, w, h))

ratios = [0.25, 0.75]
for arr in range(2):
    for i in range(2):
        compare("spotAttenMap%d_r%d.raw" % (arr, i), 32, 32,
                lambda x, y, w, h, arr=arr, r=ratios[i]: spot(r, arr, x, y, w, h))

# font
font = rd(os.path.join(D, "procfont.bin"))
dump = rd(os.path.join(D, "ref_data", "texFont8x13.raw"))
cpu = bytearray(128 * 128 * 4)
for ch in range(0x20, 0x7F):
    x0 = 8 * (ch & 0xF)
    y0 = 13 * (ch // 16)
    for r in range(13):
        for c in range(8):
            v = 0xFFFFFFFF if (font[(ch - 0x20) * 13 + r] & (0x80 >> c)) else 0
            struct.pack_into("<I", cpu, ((y0 + r) * 128 + x0 + c) * 4, v)
diff = sum(1 for a, b in zip(cpu, dump) if a != b)
print("%-32s diff bytes %d / %d" % ("texFont8x13.raw", diff, len(dump)))

mouse = rd(os.path.join(D, "promouse.bin"))
mdump = rd(os.path.join(D, "ref_data", "texMouse16x16.raw"))
print("texMouse16x16 equal: ", mouse == mdump)

white = rd(os.path.join(D, "ref_data", "texWhite1x1.raw"))
print("texWhite1x1 bytes:", white.hex())

sc = rd(os.path.join(D, "ref_data", "texShadowClip1x2.raw"))
print("texShadowClip1x2 bytes:", sc.hex())
