#pragma once

#include <Glacier/ZItemContainer.h>
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
        int m_fieldC8;
        int m_fieldCC;
    };
}