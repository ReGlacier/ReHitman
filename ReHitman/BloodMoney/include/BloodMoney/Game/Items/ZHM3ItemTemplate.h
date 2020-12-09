#pragma once

#include <Glacier/ZItemTemplate.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemTemplate : public Glacier::ZItemTemplate
    {
    public:
        // vftable
        virtual EHM3ItemType GetHM3ItemType();

        // data (total size is 0x94, ZItemTemplate size is 0x74)
        EHM3ItemType m_itemType;
        int m_field78;
        int m_field7C;
        int m_field80;
        int m_field84;
        int m_field88;
        int m_field8C;
        int m_field90;
    };
}