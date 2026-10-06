#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Render/Sprite/SSpriteArrayElementParticle.h>
#include <Glacier/Render/Sprite/SSpriteArrayParticle.h>
#include <Glacier/Runtime/ZTFixedAlloc.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/GlacierFWD.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @brief Glacier particle controller (PC zparticle.cpp).
     *
     * Reversed from the PC build: class id 0x2000E4, size 0x6E4. It owns the
     * registered particle templates (RegisterParticleTemplate) and the fixed
     * pool of particle blocks that CreateParticle writes into.
     */
    class ZParticleController : public ZSTDOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZParticleController, 0x2000E4u);

        enum : uint32_t
        {
            PARTICLES_PER_BLOCK = 0x40,
            MAX_NUM_TEMPLATES = 0xC4,
        };

        // One registered template plus the 1-based index of the block that
        // currently holds its particles (0 = none).
        struct SParticleManager
        {
            ZREF pParticleTemplate; // ZREF; the console header types this as ZParticleTemplate*
            uint32_t lBlocks;
        };

        // Collision bookkeeping entry appended by the PC CreateParticle.
        struct SCollisionEntry
        {
            uint32_t lRef;
            float fTime;
        };

        // Payload sent with the "Activate" class message.
        struct SCreateParticle
        {
            uint32_t lMagicNum;
            uint32_t lTableIndex;
            float vPos[3];
        };

        // A pool element: its own template index, the live particle count and
        // the 64 particles it can hold, followed by the shared sprite array.
        struct SParticleBlock
        {
            uint32_t lNext;      // 0x00
            uint32_t lCount;     // 0x04
            float fNextCheckTime;// 0x08
            SSpriteArrayElementParticle SpriteElements[PARTICLES_PER_BLOCK];
            SSpriteArrayParticle SpriteArray;
        };

        // methods
        ZParticleController(const char* psName, ZBaseGeom* pBaseGeom);

        // vtbl (ZGEOM/ZItem overrides)
        ~ZParticleController() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CopyData(const ZGEOM* Source) override;
        int32_t ClassCommand(ZMSGID Msg, void* pData) override;

        // Appended virtuals. PC vtable slots 118 (RegisterParticleTemplate) and
        // 119 (CreateParticle) -- see ZParticleTemplate::GetController, which
        // calls slot 118 directly.
        virtual uint32_t RegisterParticleTemplate(ZREF rTemplate);
        virtual void CreateParticle(uint32_t lTableIndex, const float* pPos, const float* pDir, TIMETYPE time);

        // data
        SParticleManager m_ParticleManager[MAX_NUM_TEMPLATES]; // 0x010
        ZTFixedAlloc<SParticleBlock, PARTICLES_PER_BLOCK>* m_pParticleBlocks; // 0x630
        uint8_t m_Unknown634[0x24]; // 0x634 (not touched by the PC ctor)
        ZMSGID m_msgActivate;       // 0x658
        uint16_t m_pad65A;          // 0x65A
        uint32_t m_lMemoryUsage;    // 0x65C
        uint32_t m_lCollisionCount; // 0x660
        SCollisionEntry m_Collisions[16]; // 0x664 .. 0x6E4
    };
    RE_VERIFY_SIZE(ZParticleController, 0x6E4);
}
