#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialInstance
    {
    public:
        // constants
        static constexpr int MAX_MATERIAL_BINDER_LISTS_NR = 18;

        // vtbl
        virtual ~ZRenderMaterialInstance();
        virtual void Begin(ZRenderContext* pRenderContext) = 0;
        virtual void Draw(ZRenderObjectInstance** ppInstances, uint32_t lCount, ZRenderContext* pRenderContext) = 0;
        virtual void End() = 0;
        virtual ZRenderObject* CreateRenderObject(const ZPrimHandle& hPrim) = 0;
        virtual void FreeRenderObject(ZRenderObject* pObject) = 0;
        virtual void PrintInfo();

        // methods
        ZRenderMaterialInstance(const char* pszName, ZRenderMaterialSubClass* pSubClass, SRMaterialProperties* pMatProperties, uint32_t lMaterialId);

        // members
        const char* m_pszName { nullptr };
        ZRenderMaterialSubClass* m_pMaterialSubClass{ nullptr };
        union
        {
            const SRMaterialProperties* m_pMaterialProperties;
            uint32_t m_lMaterialDescriptionID { 0u };
        };
        ZRenderMaterialBinderList* m_pMaterialBinderList[MAX_MATERIAL_BINDER_LISTS_NR]{}; // Approved by PC
        uint16_t m_lMaterialId { 0u };
        uint16_t m_lRemapValue { 0u };
        uint32_t m_lLayerMask { 0u };
    };
    RE_VERIFY_SIZE(ZRenderMaterialInstance, 0x60); // Verified PC allocation
}