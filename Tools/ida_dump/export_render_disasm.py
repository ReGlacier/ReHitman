"""
Export precise disassembly of the PC render 'map generator' functions (and their
leaf helpers) from an IDA database.

Purpose
-------
The Hex-Rays pseudocode of these functions is register/stack mangled (__usercall,
NEON / x87 leftovers), so a literal scalar x86 disassembly with decoded float
constants is needed to reconstruct the pixel math exactly. This script prints, for
every instruction of the requested functions:

  * the raw bytes,
  * the disassembly line,
  * for every operand that points into .rdata/.data: the resolved value decoded as
    u32/i32/f32 (4 bytes), f64 (8 bytes), or 4x f32 (16 bytes),
  * for call/jmp operands that hit another function: its (user) name,
  * for immediate operands: u32/i32/f32 interpretations.

The function set is expanded automatically: every function called (near call/jmp)
from the already collected ones is appended (breadth-first), so the whole tree of
helpers lands in one file.

Usage
-----
Run inside IDA on the PC database (ida -S or File > Script file), then inspect the
generated text file (default: <database dir>/<database>_render_disasm.txt). The
function list below can be adjusted to any database (addresses are only used as a
fallback when name lookup fails).
"""

import ida_bytes
import ida_funcs
import ida_name
import ida_segment
import ida_ua
import idc

import os
import struct

# ---------------------------------------------------------------------------
# Configuration --------------------------------------------------------------
# ---------------------------------------------------------------------------

# Roots: function names first, fallback absolute addresses.
PC_FUNCTIONS = [
    # map generators
    0x0048C210,  # CreateScatterMap
    0x0048D8D0,  # CreateSpotAttenuationMap
    0x0048DB90,  # CreateAnisotropicMap
    0x0048DD80,  # CreateSpecularMap
    0x0048DFE0,  # CreatehgPhaseMap
    0x0048E350,  # CreateRefractionMap
    0x0048E580,  # CreateNormalizerMap
    # shared resource allocation (procedural texture / font raster block)
    0x0048A160,  # ZSharedResourcesD3D::Allocate
    # extra scalar helpers reached by name if present
    "sub_48C0F0",
    "sub_48D7E0",
    "sub_48DCE0",
    "sub_48DEB0",
    "sub_48E150",
    "sub_48E0E0",
]

MAX_FUNCTIONS = 48          # breadth-first budget
FLOW_OPS = set([ida_ua.o_near, ida_ua.o_far])  # code references

# Raw .rdata blobs additionally exported next to the text dump
# (procedural texture source data of ZSharedResourcesD3D::Allocate).
DATA_RANGES = [
    (0x007F5ED8, 1235, "procfont.bin"),   # 8x13 glyphs, ASCII 0x20..0x7E (95 * 13 bytes)
    (0x007F6938, 1024, "promouse.bin"),   # 16x16 mouse cursor pixels (RGBA dwords)
]

# ---------------------------------------------------------------------------
# Decoders -------------------------------------------------------------------
# ---------------------------------------------------------------------------


def decode_word(ea):
    """Return a small dict of interpretations for the 4 bytes at ea."""
    if ea == ida_idaapi.BADADDR:
        return None
    u32 = ida_bytes.get_dword(ea)
    s32 = struct.unpack("<i", struct.pack("<I", u32))[0]
    f32 = struct.unpack("<f", struct.pack("<I", u32))[0]
    note = "u32=0x%08X i32=%d" % (u32, s32)
    try:
        if f32 == f32 and f32 not in (float("inf"), float("-inf")):
            note += " f32=%r" % f32
    except Exception:
        pass
    return note


def decode_dword_ea(ea):
    note = decode_word(ea)
    if note:
        return "[0x%X: %s]" % (ea, note)
    return None


def decode_mem(ea):
    """Decode the data at ea according to how many bytes are sensible."""
    if ea == ida_idaapi.BADADDR:
        return None
    if not ida_bytes.is_loaded(ea):
        return None
    f32 = struct.unpack("<f", struct.pack("<I", ida_bytes.get_dword(ea)))[0]
    # xmmword-sized references -> 4 floats
    if ida_bytes.is_dword(ea) and ida_bytes.is_dword(ea + 4) and ida_bytes.is_dword(ea + 8) and ida_bytes.is_dword(ea + 12):
        f4 = [struct.unpack("<f", struct.pack("<I", ida_bytes.get_dword(ea + 4 * i)))[0] for i in range(4)]
        return "[0x%X: 4xf32=%r]" % (ea, f4)
    # qword-sized -> double
    if ida_bytes.is_qword(ea) and ida_bytes.is_qword(ea + 8):
        f64 = struct.unpack("<d", struct.pack("<Q", ida_bytes.get_qword(ea)))[0]
        return "[0x%X: f64=%r]" % (ea, f64)
    return decode_dword_ea(ea)


def func_label(ea):
    f = ida_funcs.get_func(ea)
    if not f:
        return None
    name = ida_funcs.get_func_name(f.start_ea)
    return (f.start_ea, name)


# ---------------------------------------------------------------------------
# Collection -----------------------------------------------------------------
# ---------------------------------------------------------------------------


def resolve_name_or_addr(item):
    if isinstance(item, str):
        ea = ida_name.get_name_ea(ida_idaapi.BADADDR, item)
        if ea != ida_idaapi.BADADDR:
            return ea
        # maybe it is "sub_48C0F0" style; try parsing
        try:
            return int(item.replace("sub_", ""), 16)
        except Exception:
            return ida_idaapi.BADADDR
    return item


def function_bounds(ea):
    f = ida_funcs.get_func(ea)
    if not f:
        return None, None
    return f.start_ea, f.end_ea


def collect_targets(start, end, seen):
    """Yield code-flow targets inside collected funcs -> append them."""
    ea = start
    while ea < end:
        for opno in (0, 1):
            t = idc.get_operand_value(ea, opno)
            if t == ida_idaapi.BADADDR:
                continue
            if idc.get_operand_type(ea, opno) in (ida_ua.o_near, ida_ua.o_far):
                if t not in seen:
                    yield t
        ea = idc.next_head(ea, end)


def main():
    roots = []
    for item in PC_FUNCTIONS:
        ea = resolve_name_or_addr(item)
        f = ida_funcs.get_func(ea)
        if f:
            roots.append(f.start_ea)

    seen = set()
    queue = list(roots)
    ordered = []
    while queue and len(ordered) < MAX_FUNCTIONS:
        ea = queue.pop(0)
        if ea in seen:
            continue
        start, end = function_bounds(ea)
        if not start:
            continue
        seen.add(ea)
        ordered.append((start, end))
        for t in collect_targets(start, end, seen):
            if t not in seen:
                queue.append(t)

    db = idc.get_root_filename()
    base_dir = os.path.dirname(idc.get_input_file_path())
    out_path = os.path.join(base_dir, db + "_render_disasm.txt")
    with open(out_path, "w", encoding="utf-8") as f:
        f.write("# disasm export: %s\n" % db)
        for start, end in ordered:
            name = ida_funcs.get_func_name(start)
            f.write("\n\n=== %s (0x%08X - 0x%08X) ===\n" % (name, start, end))
            ea = start
            while ea < end:
                line = "0x%08X" % ea
                # raw bytes
                raw = ida_bytes.get_bytes(ea, ida_bytes.get_item_size(ea))
                if raw is not None:
                    line += "  %-18s" % raw.hex()
                line += "  %s" % idc.generate_disasm_line(ea, 0)
                notes = []
                for opno in (0, 1):
                    otype = idc.get_operand_type(ea, opno)
                    v = idc.get_operand_value(ea, opno)
                    if otype == ida_ua.o_imm:
                        w = decode_word(v)
                        if w:
                            notes.append("imm:%s" % w)
                    elif otype == ida_ua.o_mem:
                        m = decode_mem(v)
                        if m:
                            notes.append("data%s" % m)
                    elif otype in FLOW_OPS:
                        lab = func_label(v)
                        if lab:
                            notes.append("-> %s" % lab[1])
                if notes:
                    line += "   ; " + " ".join(notes)
                f.write(line + "\n")
                ea = idc.next_head(ea, end)

    for ea, size, name in DATA_RANGES:
        raw = ida_bytes.get_bytes(ea, size)
        if raw is None:
            continue
        data_path = os.path.join(base_dir, name)
        with open(data_path, "wb") as df:
            df.write(raw)
        print("data -> %s" % data_path)

    print("done -> %s" % out_path)


try:
    import ida_idaapi  # noqa: E402
    import ida_ida
except ImportError:
    # Allow this module to be imported outside IDA (syntax check only)
    import sys

    sys.stderr.write("not running inside IDA; import-only check\n")
    raise SystemExit(0)

main()
