#pragma once

#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZBaseConRout.h>
#include <Glacier/ZSTL/ZOldTypeInfo.h>

#include <string_view>


namespace Glacier
{
	template <typename T>
	struct ZFactory
	{
		static ZOldTypeInfo* Find(const char* psRequiredTypeName)
		{
			ZOldTypeInfo** factory = T::GetFactory();

			if (!factory) {
				return nullptr;
			}

			for (int i = 0; i < 0xF; i++) {
				Glacier::ZOldTypeInfo* currentTypeInfo = factory[i];
				if (!currentTypeInfo) {
					continue;
				}

				do {
					const std::string_view psName = currentTypeInfo->m_psName;

					if (psName == psRequiredTypeName) {
						return currentTypeInfo;
					}

					currentTypeInfo = currentTypeInfo->m_pNext;
				} while (currentTypeInfo);
			}

			return nullptr;
		}
	};
}