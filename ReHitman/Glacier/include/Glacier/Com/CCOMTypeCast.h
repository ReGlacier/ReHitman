#pragma once

#include <Glacier/Com/CCOMType.h>

namespace Glacier
{
	struct CCOMTypeCast
	{
		union 
        {
			int lLetters;
			bool bLetters[4];
		};

		int lTypeLen;
		CCOMType eComType;
		int eLen;
		int eCComFormat;
	};
}