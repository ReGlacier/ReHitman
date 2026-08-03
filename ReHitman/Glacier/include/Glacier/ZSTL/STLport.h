#pragma once

// STLport 4.6.1 expresses its own namespace through the historical _STL
// macro. Keep that macro scoped to this adapter because modern MSVC also uses
// _STL internally for namespace std.
#define _STL stlp
#define _STLP_DONT_THROW_RANGE_ERRORS 1
#define _STLP_NO_EXTENSIONS 1

#include <stlport/map>
#include <stlport/set>
#include <stlport/vector>

#undef std
#undef _STL
