#include <Glacier/EventBase/ZBaseConRout.h>
#include <G1ConfigurationService.h>


namespace Glacier
{
	ZBaseConRout::~ZBaseConRout()
	{
		auto* pRoutClassInfo = GetRoutClassInfo();
		assert(pRoutClassInfo);

		// Do nothing else, but in PS2 build we had memory tracking logic
	}

	ZBaseConRout::ZBaseConRout() 
		: ZEventBase()
	{
	}

	void ZBaseConRout::InitBaseConRout(ZROUTCLASSINFO* pRoutClassInfo)
	{
		m_pRoutClassInfo = pRoutClassInfo;
		m_lRoutCases = pRoutClassInfo->RoutCases();
	}

	int32_t ZBaseConRout::UnknownCommand(ZMSGID command, ZDATA data)
	{
		return 0;
	}

	const char* ZBaseConRout::EventName()
	{
		if (auto* pRoutClassInfo = GetRoutClassInfo())
		{
			return pRoutClassInfo->RoutName();
		}

		return nullptr;
	}

	int ZBaseConRout::DoEvent(int lType, uint16_t lParam, void* pData)
	{
		switch (lType)
		{
			case 0x8:
			{
				TimeUpdate();
			}
			break;
			case 0x10:
			{
				FrameUpdate();
			}
			break;
			case 0x20:
			{
				Command(lParam, pData);
			}
			break;
			case 0x1000:
			{
				SchedUpdate();
			}
			break;
		}

		return 0;
	}

	ZFactory<ZBaseConRout>& ZBaseConRout::GetFactory()
	{
		return ZBaseConRout::m_Factory;
	}

	ZROUTCLASSINFO* ZBaseConRout::GetRoutClassInfo() const
	{
		return m_pRoutClassInfo;
	}

	STATIC_CLASS_VAR_IMPL(ZBaseConRout, ZFactory<ZBaseConRout>, m_Factory, 0x00A4F8F8, ZFactory<ZBaseConRout>{});
}