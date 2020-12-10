#pragma once

#include <Glacier/Items/ZItem.h>

namespace Glacier
{
    class ZItemAmmo : public ZItem
    {
    public:
        // vftable
        virtual int GetNrProjectiles();
        virtual int SetNrProjectiles(int value);
        virtual void AddNrProjectiles(int amount);
        virtual void SubNrProjectiles(int amount);

        // data (total size is 0x88, ZItem size is 0x84)
        int m_iNrOfProjectiles;
    };
}