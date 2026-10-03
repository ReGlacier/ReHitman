#pragma once

#include <Glacier/Com/CCOMTypeCast.h>


namespace Glacier
{
    struct ComValueInfo
	{
		int lNameLen;
		CCOMTypeCast* lType;
		int lDataLen;
	};
}