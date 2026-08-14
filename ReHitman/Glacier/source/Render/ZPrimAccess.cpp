#include <Glacier/Render/ZPrimAccess.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/Material/ZRenderMaterialSubClass.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    void ZPrimAccess::Destroy()
    {
        if ((m_lStatusFlags & 2) == 0)
        {
            ZUniMemory::Delete(this);
        }
    }

    void ZPrimAccess::Lock(uint32_t lLockFlags)
    {
        ZASSERT((m_lStatusFlags & 1) == 0);

        m_lStatusFlags |= 1u;
    }

    void ZPrimAccess::Unlock()
    {
        ZASSERT((m_lStatusFlags & 1) != 0);

        m_lStatusFlags &= ~1u;
    }

    void ZPrimAccess::ReleasePrim()
    {
        // Do nothing
    }

    ZPrimAccess::~ZPrimAccess() = default;

    ZPrimAccess* ZPrimAccess::Create(const ZPrimHandle& hPrim)
    {
        void* pPrimEntry = nullptr;

        if (!(hPrim.m_lHandleValue & 0x80000000))
        {
            pPrimEntry = g_apPrimHandleToPointerTable[hPrim.m_lHandleValue];
        }
        else
        {
            pPrimEntry = (void*)(hPrim.m_lHandleValue & 0x7FFFFFFFu);
        }

        const auto arg = *reinterpret_cast<uint16_t*>(reinterpret_cast<int32_t>(pPrimEntry) + 18); // <<< TODO: Make me better
        
        ZASSERT(g_pMaterialBufferInstance);
        auto* pMaterialInstance = g_pMaterialBufferInstance->GetMaterialInstance(arg);
        if (!pMaterialInstance)
        {
            return nullptr;
        }

        return pMaterialInstance->m_pMaterialSubClass->CreatePrimAccess(hPrim);
    }
}