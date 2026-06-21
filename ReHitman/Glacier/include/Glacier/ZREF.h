#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier
{
	// This class will resolve any Glacier reference to ZGEOM object
	// struct ZREF
	// {
	// 	unsigned int rRef{};

	// 	ZREF() = default;
	// 	ZREF(int v) : rRef(static_cast<unsigned int>(v)) {}
	// 	ZREF(unsigned int v) : rRef(v) {}

	// 	operator bool() const { return rRef > 0u; }
	// 	bool operator!() const { return rRef > 0u; }
	// };
	// RE_VERIFY_SIZE(ZREF, 0x4);

	// We will return back that strucutre, but now it breaks everything
	using ZREF = unsigned int;
}