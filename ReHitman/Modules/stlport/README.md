# STLport 4.6.1

Source: https://sourceforge.net/projects/stlport/files/STLport%20archive/STLport%204/

This directory vendors the original STLport 4.6.1 source package for binary-compatible reverse engineering work. Glacier PC/PS2 binaries use STLport-style containers (`std::_Rb_tree`, `_STLP_alloc_proxy`, etc.), and PS2 debug paths reference `libraries/STLport-4.6.1/stlport/stl/_iterator.h`, so this version is used as the canonical STL implementation for original engine STL layouts.

What is used by the build:

- `STLport-4.6.1/stlport/` is used as the STLport header tree.
- The CMake target is named `stlport`.
- STLport is configured to expose containers in the `stlp` namespace, not `std`, so reversed Glacier code can explicitly distinguish original STLport containers from the host C++ standard library.
- `STLportLayoutProbe` verifies key container sizes in CI, including `stlp::string`, `stlp::vector`, `stlp::map`, `stlp::multimap`, and `stlp::set`.

Compatibility staging:

- STLport 4.6.1 expects an old compiler install layout where `stlport/` and native `include/` are sibling directories.
- The CMake target creates a generated staging directory under the build tree and copies STLport headers plus the MSVC/UCRT native headers into that layout.
- The vendored STLport source files are not patched for this staging behavior.

Copyright and license:

- STLport is third-party code and keeps its original copyright/license notices in the imported files.
- ReHitman does not claim authorship of STLport.
- Local project files in this module only describe how the vendored package is wired into this build and how its layouts are verified.
