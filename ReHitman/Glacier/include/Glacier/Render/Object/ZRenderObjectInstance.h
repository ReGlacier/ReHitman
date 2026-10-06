#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Render/Fwd.h>
#include <type_traits>
#include <cstdint>


namespace Glacier
{
    class ZRenderObjectInstance
    {
    public:
        // vtbl
        virtual ~ZRenderObjectInstance();
        virtual void Draw(ZRenderContext* pCtx) = 0;
        virtual void UpdateLight(const SUpdateLightData* pUpdateLightData, uint32_t lNumLights);

        // methods
        ZRenderObjectInstance(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom);

        template <typename T>
        T* RenderObject() requires (std::is_base_of_v<ZRenderObject, T>)
        {
            return reinterpret_cast<T*>(m_pRenderObject);
        }
        
        // members
        ZRenderObject* m_pRenderObject; // +0x4
        ZBaseGeom* m_pBaseGeom; // +0x8
        ZRenderEntry* m_pRenderEntry; // +0xC
        uint32_t m_lSortValue; // +0x10
        uint32_t m_lFlags; // +0x14
    };
}