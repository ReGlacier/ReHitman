#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/ZPrimHandle.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderObject
    {
    public:
        // vtbl
        virtual ~ZRenderObject();
        virtual ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) = 0;
        virtual void Update();

        // methods
        ZRenderObject(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);

        // members
        ZRenderMaterialInstance* m_pMaterialInstance; // +0x4
        ZPrimHandle m_hPrim; // +0x8
        uint16_t m_lNumActiveInstances; // +0xC
        uint16_t m_lFlags; // +0x10
    };
}