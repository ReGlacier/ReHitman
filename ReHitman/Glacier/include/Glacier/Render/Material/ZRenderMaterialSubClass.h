#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialSubClass
    {
    public:
        // constants
        static constexpr int MAX_LAYERS_NR = 18; // Approved by ZRenderMaterialSubClass::Initialize & ZRenderMaterialSubClassD3D::CreateMaterialInstance

        // vtbl
        virtual ~ZRenderMaterialSubClass();
        virtual void Initialize();
        virtual uint32_t Begin(uint32_t lLayerIndex, const ZRenderContext* pCtx);
        virtual void End();
        virtual void BeginPass(uint32_t lPassIndex, const ZRenderContext* pCtx);
        virtual void EndPass();
        virtual void BeginInstance(const ZRenderMaterialInstance* pMaterial, const ZRenderContext* pCtx);
        virtual void EndInstance();
        virtual void Draw(const ZRenderMaterialInstance* pMaterial, ZRenderObjectInstance** ppObjects, uint32_t lObjectsNr, ZRenderContext* pCtx) = 0;
        virtual ZRenderMaterialInstance* CreateMaterialInstance(const SRMaterialProperties* pProperties, uint32_t lMaterialId) = 0;
        virtual ZRenderObject* CreateRenderObject(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterial) = 0;
        virtual ZPrimAccess* CreatePrimAccess(const ZPrimHandle& hPrim);
        virtual uint32_t GetPrimAccessSize(const ZPrimHandle& hPrim);
        virtual void CreatePrimAccessInplace(ZPrimAccess* pWhere, const ZPrimHandle& hPrim);

        // methods
        ZRenderMaterialSubClass(const char* pszName, ZRenderMaterialClass* pMaterialClass, uint16_t lObjectType, uint16_t lObjectSubType, uint32_t lMaterialFlags);

        // members
        const char* m_pszName { nullptr };
        ZRenderMaterialClass* m_pMaterialClass { nullptr };
        ZRenderMaterialLayer* m_pLayers[MAX_LAYERS_NR] { nullptr }; // Verified in PC ctor
        uint32_t m_lMaterialFlags { 0u };
        uint16_t m_lObjectType { 0u };
        uint16_t m_lObjectSubType { 0u };
        uint16_t m_lIndex { 0u };
        uint8_t m_lLayer { 0u };
        uint8_t m_lPass { 0u };
    };
}