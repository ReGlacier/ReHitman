#include <Glacier/System/ZSysMem.h>
#include <Glacier/Debug/ZMemReadOut.h>


namespace Glacier
{
    namespace
    {
        static uint32_t g_iMemColor = 0;

        uint32_t ZDebugSetMemColor(uint32_t lColor)
        {
            auto lOldMemColor = g_iMemColor;
            g_iMemColor = lColor;
            return lOldMemColor;
        }
    }

    // Methods here

    template<>
    ISysMem* ZComponentSingleton<ISysMem, ZGlobalComponentBase>::m_pInstance = reinterpret_cast<ISysMem*>(0x008208C8); // GoG version

    void* ISysMem::New(EAllocType eMemType, int iSize)
    {
        using ZSysMem_New_t = void*(__thiscall*)(ISysMem*, EAllocType, int);
        ZSysMem_New_t Impl = (ZSysMem_New_t)0x004467B0;
        return Impl(this, eMemType, iSize);
    }

    void ISysMem::Delete(void* pMem)
    {   
        using ZSysMem_Delete_t = void(__thiscall*)(ISysMem*, void*);
        ZSysMem_Delete_t Impl = (ZSysMem_Delete_t)0x004468B0;
        Impl(this, pMem);
    }

    uint32_t SetMemColor(uint32_t lNewColor)
    {
        auto& pMemReadOut = ZMemReadOut::Instance();

        auto lOldColor = pMemReadOut.SetAllocColor(lNewColor);
        ZDebugSetMemColor(lNewColor);
        
        return lOldColor;
    }
}
