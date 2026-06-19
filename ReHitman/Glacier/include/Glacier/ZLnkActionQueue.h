#pragma once

#include <Glacier/ZActionDispatcher.h>


namespace Glacier
{
	class LINKREFTAB;

	struct ZLnkActionQueue : public ZActionDispatcher
	{
		// It's weird, in PC it's LINKREFTAB, but on PS2 it's REFTAB. Idk why lol
		// In PC see ZLnkActionQueue::Execute for details (0x00653C20)
		LINKREFTAB* m_pActionQueue;

		virtual bool CheckActionQueueForId(unsigned int id);
	};
	RE_VERIFY_SIZE(ZLnkActionQueue, 0x10);
}