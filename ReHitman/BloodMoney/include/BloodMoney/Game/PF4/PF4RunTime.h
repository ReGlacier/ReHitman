#pragma once

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
}