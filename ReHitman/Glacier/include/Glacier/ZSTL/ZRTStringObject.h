#pragma once

#include <Glacier/ZListNodeBase.h>
#include <cstdint>


namespace Glacier
{
	struct ZStringMemoryManager
	{
		struct ZHeader
		{
			struct ZRefAndSize
			{
				uint32_t m_Data;
			};

			ZRefAndSize m_RefOrSize;
		};
	};

	struct ZRTStringObject : public ZStringMemoryManager::ZHeader, public ZListNode<ZRTStringObject, 0>
	{
	};
	RE_VERIFY_SIZE(ZRTStringObject, 0xC);

	struct ZRTString
	{
		ZRTStringObject* m_StringObject;
	};
}