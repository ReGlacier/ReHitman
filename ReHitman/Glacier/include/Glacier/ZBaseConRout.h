#pragma once

#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZSTL/ZOldTypeInfo.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/ZNonResourceClassInfo.h>
#include <Glacier/Runtime/ZROUTCLASSINFO.h>
#include <Glacier/Runtime/ZFactory.h>


namespace Glacier
{
	class ZBaseConRout : public ZEventBase
	{
	public:
#		pragma region " --- ZFactory<T> stuff --- "
		struct ZRoutCreator
		{
            struct Create
            {
				template <typename T>
                static T* Do(const ZROUTCLASSINFO& sRoutClassInfo)
                {
					auto* pEvent = ZEventBuffer::Instance().AllocEventRam(sizeof(T));
					if (pEvent)
					{
						// init
						ZBaseConRout* pInstance = znew_placement<T>(pEvent);
						pInstance->InitBaseConRout(sRoutClassInfo);
					}

					return pEvent;
                }
            };
		};

		DECLARE_FACTORY(ZROUTCLASSINFO, const char*, ZRoutCreator);
#		pragma endregion
		
		// static vars
		STATIC_CLASS_VAR(ZBaseConRout, ZFactory<ZBaseConRout>, m_Factory);

		// vtbl
		~ZBaseConRout() override;
		virtual void InitBaseConRout(ZROUTCLASSINFO* pBaseConRout);
		virtual int32_t UnknownCommand(ZMSGID command, ZDATA data);
		const char* EventName() override;
		int DoEvent(int lType, uint16_t lParam, void* pData) override;

		// methods
		ZBaseConRout();
		static ZFactory<ZBaseConRout>& GetFactory();
		ZROUTCLASSINFO* GetRoutClassInfo() const;

		// members
		ZROUTCLASSINFO* m_pRoutClassInfo;
	};
	RE_VERIFY_SIZE(ZBaseConRout, 0x30); // Verified
	RE_VERIFY_OFFSET(ZBaseConRout, m_pRoutClassInfo, 0x2C);
}