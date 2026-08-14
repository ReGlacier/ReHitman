#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialBufferD3D : public ZRenderMaterialBuffer
    {
    public:
        // constants
        static constexpr int MAX_MATERIAL_INSTANCES_NR = 2048; // Verified PC
        static constexpr int MAX_MATERIAL_CLASSES_NR = 32; // Verified PC

        // vtbl
        ~ZRenderMaterialBufferD3D() override;
        void AllocateResources() override;
        void FreeResources() override;
        ZRenderMaterialInstance* GetMaterialInstance(uint32_t lMaterialId) override;
        uint32_t CreateMaterialInstanceSprite(uint32_t lTextureId, uint32_t lDrawMode, uint32_t lSpriteType) override;
        uint32_t GetTextureFromMaterial(uint32_t lMaterialId, const char* pszTextureBinderName) override;
        uint32_t GetAllTexturesFromMaterial(uint32_t lMaterial, uint32_t* ppTextures, uint32_t lMaxTexturesNr) override;
        uint32_t GetTextureAnimData(uint32_t lMaterialId) override;
        virtual uint32_t NumMaterialInstances() const;

        // methods
        ZRenderMaterialBufferD3D();

        void CreateMaterials();
        ZRenderMaterialClass* CreateMaterialClass(uint32_t lMaterialClass, const SRMaterialProperties* pProperties);
        void FreeSubClasses();

        // members
        uint8_t m_D3DSpecificShit[8192]; // TODO: Finish me
        uint32_t m_lNumMaterialClasses { 0u };
        ZRenderMaterialClass* m_pMaterialClasses[MAX_MATERIAL_CLASSES_NR];
        int m_lNumMaterialInstances{ 0 };
        ZRenderMaterialInstance* m_pMaterialInstances[MAX_MATERIAL_INSTANCES_NR];
        bool m_bResourcesAllocated { false };
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZRenderMaterialBufferD3D, 0x40B0);
}