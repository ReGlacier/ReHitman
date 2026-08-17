#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Fysix/ZDynamicsExtend.h>
#include <cstdint>


namespace Glacier
{
    struct SVertexMapper : SGenericMapper
    {
        uint16_t m_wVertexCount;
        uint16_t* m_pIndices;
    };

    class ZVertexExtend : public ZDynamicsExtend
    {
    public:
        // vtbl
        ~ZVertexExtend() override;
        bool Update(uint16_t body, ZGEOM* geom) override;
        bool CreateMapper(const SGenericMapper* mpr, uint16_t mappings) override;
        void DestroyMapper() override;
        const REFTAB* GetCollisionFaces(uint16_t body, ZGEOM* geom) override;
        bool Collision(uint16_t body, ZGEOM* geom) override;
        bool Fracture(uint16_t body, ZGEOM* geom) override;

        // methods
        ZVertexExtend();
    };

    RE_VERIFY_SIZE(ZVertexExtend, 0x58);
}