"""
Dump ZSharedResourcesD3D-related globals from a running HitmanBloodMoney.exe.

Usage:
    python dump_process_mem.py                # auto-find by process name
    python dump_process_mem.py 1234           # by PID
    python dump_process_mem.py <pid> <outdir>

Reads the regions listed in REGIONS below (absolute RVAs of the PC build),
resolves them against the loaded module base (+ PE ImageBase) and writes one
.bin per region next to the script (or into <outdir>). Works for a running
menu/scene - the shared resources are created once at renderer init.

Run it while the game is running (menu is enough).
"""

import ctypes
import ctypes.wintypes as wt
import os
import struct
import sys

PROCESS_VM_READ = 0x0010
PROCESS_QUERY_INFORMATION = 0x0400
TH32CS_SNAPPROCESS = 0x00000002
TH32CS_SNAPMODULE = 0x00000008
INVALID_HANDLE_VALUE = ctypes.c_void_p(-1).value

PROCESS_NAME = "HitmanBloodMoney.exe"

# (file name, RVA, size) - absolute RVAs of the PC build.
REGIONS = [
    ("font_letter_positions.bin", 0x008EE130, 0x400),  # g_FontLetterPositions floats

    ("g_texRefractionMap.bin",     0x0090AC18, 0x44),
    ("g_texShadowCubeMapColor.bin",0x0090AC60, 0x44),
    ("g_texNormalizer.bin",        0x0090ACA8, 0x44),
    ("g_texShadowMapDepth.bin",    0x0090AD80, 0x44),
    ("g_texWhite.bin",             0x0090ADC8, 0x44),
    ("g_texMouse16x16.bin",        0x0090AE10, 0x44),
    ("g_texFont8x13.bin",          0x0090AE58, 0x44),
    ("g_texShadowClip1x2.bin",     0x0090AEA0, 0x44),

    # spot/shadow colour map slot arrays (texture handles live at offset 0 of
    # each 0x44 element)
    ("g_pSpotAttenuationMap.bin",  0x0090AA14, 0x88),
    ("g_pSpotAttenuationMap2.bin", 0x0090A98C, 0x88),
    ("g_texShadowMapColor.bin",    0x0090AB34, 0x110),

    # whole tail of the globals block, to have extra context around wrappers
    ("shared_tail.bin",            0x0090A980, 0x5C0),
]


class PROCESSENTRY32W(ctypes.Structure):
    _fields_ = [
        ("dwSize", wt.DWORD),
        ("cntUsage", wt.DWORD),
        ("th32ProcessID", wt.DWORD),
        ("th32DefaultHeapID", ctypes.POINTER(ctypes.c_ulong)),
        ("th32ModuleID", wt.DWORD),
        ("cntThreads", wt.DWORD),
        ("th32ParentProcessID", wt.DWORD),
        ("pcPriClassBase", ctypes.c_long),
        ("dwFlags", wt.DWORD),
        ("szExeFile", ctypes.c_wchar * 260),
    ]


class MODULEENTRY32W(ctypes.Structure):
    _fields_ = [
        ("dwSize", wt.DWORD),
        ("th32ModuleID", wt.DWORD),
        ("th32ProcessID", wt.DWORD),
        ("GlblcntUsage", wt.DWORD),
        ("ProccntUsage", wt.DWORD),
        ("modBaseAddr", ctypes.POINTER(ctypes.c_byte)),
        ("modBaseSize", wt.DWORD),
        ("hModule", wt.HMODULE),
        ("szModule", ctypes.c_wchar * 256),
        ("szExePath", ctypes.c_wchar * 260),
    ]


kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)

kernel32.CreateToolhelp32Snapshot.argtypes = [wt.DWORD, wt.DWORD]
kernel32.CreateToolhelp32Snapshot.restype = ctypes.c_void_p
kernel32.Process32FirstW.argtypes = [ctypes.c_void_p, ctypes.POINTER(PROCESSENTRY32W)]
kernel32.Process32FirstW.restype = wt.BOOL
kernel32.Process32NextW.argtypes = [ctypes.c_void_p, ctypes.POINTER(PROCESSENTRY32W)]
kernel32.Process32NextW.restype = wt.BOOL
kernel32.Module32FirstW.argtypes = [ctypes.c_void_p, ctypes.POINTER(MODULEENTRY32W)]
kernel32.Module32FirstW.restype = wt.BOOL
kernel32.Module32NextW.argtypes = [ctypes.c_void_p, ctypes.POINTER(MODULEENTRY32W)]
kernel32.Module32NextW.restype = wt.BOOL
kernel32.OpenProcess.argtypes = [wt.DWORD, wt.BOOL, wt.DWORD]
kernel32.OpenProcess.restype = ctypes.c_void_p
kernel32.ReadProcessMemory.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t)]
kernel32.ReadProcessMemory.restype = wt.BOOL
kernel32.CloseHandle.argtypes = [ctypes.c_void_p]


def find_process_pid(name):
    snap = kernel32.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
    if snap == INVALID_HANDLE_VALUE:
        raise RuntimeError("CreateToolhelp32Snapshot failed: %d" % ctypes.get_last_error())
    try:
        pe = PROCESSENTRY32W()
        pe.dwSize = ctypes.sizeof(pe)
        if not kernel32.Process32FirstW(snap, ctypes.byref(pe)):
            return None
        while True:
            if pe.szExeFile.lower() == name.lower():
                return pe.th32ProcessID
            if not kernel32.Process32NextW(snap, ctypes.byref(pe)):
                return None
    finally:
        kernel32.CloseHandle(snap)


def get_module_base(pid):
    """Return (base, image_base_from_pe) of the main module."""
    snap = kernel32.CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPPROCESS, pid)
    if snap == INVALID_HANDLE_VALUE:
        raise RuntimeError("CreateToolhelp32Snapshot(module) failed: %d" % ctypes.get_last_error())
    try:
        me = MODULEENTRY32W()
        me.dwSize = ctypes.sizeof(me)
        if not kernel32.Module32FirstW(snap, ctypes.byref(me)):
            raise RuntimeError("no main module")
        base = ctypes.addressof(me.modBaseAddr.contents)
        # PE ImageBase (PE32): e_lfanew at +0x3C, ImageBase at e_lfanew+52
        h = kernel32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, pid)
        if not h:
            raise RuntimeError("OpenProcess failed: %d" % ctypes.get_last_error())
        try:
            buf = ctypes.create_string_buffer(4096)
            n = ctypes.c_size_t()
            if not kernel32.ReadProcessMemory(h, ctypes.c_void_p(base), buf, 4096, ctypes.byref(n)):
                raise RuntimeError("cannot read PE header")
            e_lfanew = struct.unpack_from("<I", buf.raw, 0x3C)[0]
            if e_lfanew + 64 > n.value:
                raise RuntimeError("PE header out of the read window")
            image_base = struct.unpack_from("<I", buf.raw, e_lfanew + 52)[0]
        finally:
            kernel32.CloseHandle(h)
        return base, image_base
    finally:
        kernel32.CloseHandle(snap)


def read_memory(pid, addr, size):
    h = kernel32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, pid)
    if not h:
        raise RuntimeError("OpenProcess failed: %d" % ctypes.get_last_error())
    try:
        buf = ctypes.create_string_buffer(size)
        n = ctypes.c_size_t()
        ok = kernel32.ReadProcessMemory(h, ctypes.c_void_p(addr), buf, size, ctypes.byref(n))
        if not ok or n.value != size:
            raise RuntimeError(
                "ReadProcessMemory @0x%X size 0x%X failed: %d (read %d)"
                % (addr, size, ctypes.get_last_error(), n.value)
            )
        return buf.raw
    finally:
        kernel32.CloseHandle(h)


def main():
    args = [a for a in sys.argv[1:]]
    pid = None
    if args and args[0].isdigit():
        pid = int(args[0])
        args = args[1:]
    if not pid:
        pid = find_process_pid(PROCESS_NAME)
    if not pid:
        print("ERROR: process '%s' not found" % PROCESS_NAME, file=sys.stderr)
        return 1

    outdir = args[0] if args else os.path.dirname(os.path.abspath(__file__))
    os.makedirs(outdir, exist_ok=True)

    base, image_base = get_module_base(pid)
    print("pid=%d module_base=0x%X image_base=0x%X" % (pid, base, image_base))

    manifest = []
    for name, rva, size in REGIONS:
        try:
            data = read_memory(pid, base + (rva - image_base), size)
        except Exception as e:
            print("  FAIL %-32s %s" % (name, e))
            return 1
        path = os.path.join(outdir, name)
        with open(path, "wb") as f:
            f.write(data)
        print("  OK   %-32s %d bytes -> %s" % (name, len(data), path))
        manifest.append({"file": name, "rva": rva, "size": size})

    with open(os.path.join(outdir, "manifest.json"), "w") as f:
        import json

        json.dump(manifest, f, indent=2)
    print("done")
    return 0


if __name__ == "__main__":
    sys.exit(main())
