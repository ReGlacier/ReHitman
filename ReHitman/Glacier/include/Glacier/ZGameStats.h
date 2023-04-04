#pragma once

#include <Glacier/Glacier.h>

namespace Glacier
{
    class ZGameStats
    {
    public:
        //vftable
        virtual void Release(bool);
        virtual float GetIntensity();

        //api
        void IncreaseCurrentShotCount();
        void DecreaseCurrentShotCount();

        //data (total size is 0xC)
        unsigned short m_iShootCount;
        unsigned short m_field6;
        float m_fIntensity;
    };
}