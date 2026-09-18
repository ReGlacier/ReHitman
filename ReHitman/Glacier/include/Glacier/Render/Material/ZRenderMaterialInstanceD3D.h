#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>


namespace Glacier
{
    class ZRenderMaterialInstanceD3D : public ZRenderMaterialInstance
    {
    public:
        // vtbl
        ~ZRenderMaterialInstanceD3D() override;
        void Begin(ZRenderContext* pRenderContext) override;
        void Draw(ZRenderObjectInstance** ppInstances, uint32_t lCount, ZRenderContext* pRenderContext) override;
        void End() override;
        ZRenderObject* CreateRenderObject(const ZPrimHandle& hPrim) override;
        void FreeRenderObject(ZRenderObject* pObject) override;
        virtual bool Update(ZRMaterialObject* pObject);

        // methods
        ZRenderMaterialInstanceD3D(const char* pszName, ZRenderMaterialSubClass* pSubClass, SRMaterialProperties* pMatProperties, uint32_t lMaterialId);
    };
    RE_VERIFY_SIZE(ZRenderMaterialInstanceD3D, 0x60); // Verified PC
}