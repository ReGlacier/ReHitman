#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialLayer
    {
    public:
        // vtbl
        virtual ~ZRenderMaterialLayer() = default;
        virtual uint32_t BeginSubClass(const ZRenderMaterialSubClass* pSubClass, const ZRenderContext* pContext) = 0;
        virtual void EndSubClass() = 0;
        virtual void BeginPass(uint32_t lPassIndex, const ZRenderContext* pContext) = 0;
        virtual void EndPass() = 0;
        virtual void BeginInstance(const ZRenderMaterialInstance* pMaterialInstance, const ZRenderMaterialBinderList* pBinderList, const ZRenderContext* pContext) = 0;
        virtual void EndInstance() = 0;
        virtual void BeginObject(const ZRenderMaterialInstance* pMaterialInstance, const ZRenderMaterialBinderList* pBinderList, const ZRenderObjectInstance* pInstance, const ZRenderContext* pContext) = 0;
        virtual void EndObject() = 0;
        virtual uint32_t NumPasses() = 0;
        virtual void CreateBinders(int lIndex, ZRenderMaterialInstance* pMaterialInstance, ZRenderMaterialBinderList* pBinderList) = 0;
        virtual void Initialize()
        {
            // Do nothing
        }

        // methods
        ZRenderMaterialLayer() = default;
        
        ZRenderMaterialLayer(const char* pszName)
            : m_pszName(pszName)
        {
        }

        // members
        const char* m_pszName { nullptr }; // 0x4
    };
    RE_VERIFY_SIZE(ZRenderMaterialLayer, 0x8);
}