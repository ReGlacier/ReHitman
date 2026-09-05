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
#include <stlport/exception.h>

#undef std
#undef _STL

// stlport/exception.h only forwards to the native <exception>. The original engine ran
// STLport in its own-namespace mode, which re-exposed the standard exception base through
// the STLport namespace; restore that so engine code can use stlp::exception.
namespace stlp
{
    using std::exception;
}
