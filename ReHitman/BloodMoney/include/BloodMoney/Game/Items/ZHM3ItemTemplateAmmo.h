#pragma once

#include <Glacier/ZItemTemplateAmmo.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemTemplateAmmo : public Glacier::ZItemTemplateAmmo
    {
    public:
        //vftable
        virtual EHM3ItemType GetHM3ItemType();

        //data (total size is 0xA8, Glacier::ZItemTemplateAmmo size is 0xA4)
        EHM3ItemType m_itemType;
    };
}