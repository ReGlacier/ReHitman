#pragma once

#include <Glacier/ZItem.h>

#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    class ZHM3Item : public Glacier::ZItem
    {
    public:
        // vftable
        virtual EHM3ItemType GetHM3ItemType();
        virtual void OverrideItemType(EHM3ItemType itemType);
        virtual void UseItemActivateAnimation();

        // data (total size is 0x9C, ZItem size is 0x84)
        int m_field84;
        int m_field88;
        EHM3ItemType m_itemType;
        int m_field90;
        int m_field94;
        int m_field98;
    };
}