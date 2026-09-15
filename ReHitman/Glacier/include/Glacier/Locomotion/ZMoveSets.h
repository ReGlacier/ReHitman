#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier::Locomotion
{
    struct ZSet;

    struct ZMoveSets
    {
        static constexpr int MaxSets = 15;

        static ZSet* Get(int index);
        static void Set(int index, ZSet* pSet);

        static ZSet* m_pSets[MaxSets];
        static int m_iSize;
    };
}
