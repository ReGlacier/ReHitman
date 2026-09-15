#pragma once

#include <Glacier/ZSTL/ZMath.h>

namespace Hitman::BloodMoney::PF4RunTime {
    class ZData {
    public:
        //vtable
        virtual void Release(bool bFreeMem);

        //static
        ZData* CreatePathFinder(void* pData);
        //data (total size is 0x5E80)
        int m_field4;
        //TODO: Complete reverse here
    };


    struct ZPath
    {
        int* m_PathIdx;
        int m_iMaxSize;
        Glacier::ZVector3 m_Vertices[4];
        int m_CustomVertices;
        void* m_PathFinder;
        int m_Size;
        float m_Cost;

        void GetPosition(int index, Glacier::ZVector3& pos)
        {
            using GetPosition_t = void(__thiscall *)(ZPath*, int, Glacier::ZVector3&);

            GetPosition_t GetPositionImpl = reinterpret_cast<GetPosition_t>(0x004D8E20);
            GetPositionImpl(this, index, pos);
        }
        
        void GetEndPosition(Glacier::ZVector3& pos)
        {
            // from original code
            if (m_Size > 0)
            {
                GetPosition(m_Size - 1, pos);
            }
        }
    };
}