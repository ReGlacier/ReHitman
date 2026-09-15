#pragma once

#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/ReGlacier.h>

namespace Hitman::BloodMoney
{
	struct ZHM3CombineItems : public Glacier::ZGROUP 
	{
        Glacier::ZItemTemplate* m_pCombine1;
        Glacier::ZItemTemplate* m_pCombine2;
        int32_t m_iAllocNumber;
        int32_t m_iSpawnedNumber;
        bool m_bSpawnInInventory;
        Glacier::ZREF m_rCombine1;
        Glacier::ZREF m_rCombine2;
        Glacier::REFTAB* m_pCombineCloneGroups;
	};
	RE_VERIFY_SIZE(ZHM3CombineItems, 0x6C); // Verified
}