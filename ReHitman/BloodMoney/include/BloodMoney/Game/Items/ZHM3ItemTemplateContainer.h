#pragma once

#include <Glacier/Items/ZItemTemplateContainer.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemTemplateContainer : public Glacier::ZItemTemplateContainer
    {
    public:
        //vftable
        virtual EHM3ItemType GetHM3ItemType();

        //data (total size is 0xB4, ZItemTemplateContainer is 0x90)
        int m_field90;
        int m_field94;
        EHM3ItemType m_itemType;
        int m_field9C;
        int m_fieldA0;
        int m_fieldA4;
        int m_fieldA8;
        int m_fieldAC;
        int m_fieldB0;
    };
}