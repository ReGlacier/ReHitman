#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Items/ZItemContainer.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemContainer : public Glacier::ZItemContainer
    {
    public:
        // vftable
        virtual EHM3ItemType GetHM3ItemType();
        virtual bool IsDetectable();

        // data (total size is 0xD0, ZItemContainer size is 0xC8)
        Glacier::ZLNKOBJ* m_pGround;
        bool m_bForceUnpickable;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZHM3ItemContainer, 0xD0);
}