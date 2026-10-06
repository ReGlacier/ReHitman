#include <Glacier/Geom/ZParticleController.h>

#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZParticleTemplate.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/ZMath.h>

#include <cmath>
#include <cstdlib>


namespace Glacier
{
    // PC 0x7FC030 -- value of the "particle_collision" debug int declared in zparticle.cpp.
    static int g_lParticleCollision = 1;
    // PC 0x7FC074 / 0x972FCC -- per-frame limiter for the collision probes.
    static uint32_t g_lParticleCollisionFrame = 0;
    static int g_lParticleCollisionCount = 0;

    // PC ctor 0x4EF9B0
    ZParticleController::ZParticleController(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom)
        , m_pParticleBlocks(nullptr)
        , m_msgActivate(0)
        , m_pad65A(0)
        , m_lMemoryUsage(0)
        , m_lCollisionCount(0)
    {
        // The PC ctor zeroes [0x10, 0x630), i.e. the whole manager table.
        for (SParticleManager& manager : m_ParticleManager)
        {
            manager.pParticleTemplate = 0;
            manager.lBlocks = 0;
        }

        for (SCollisionEntry& collision : m_Collisions)
        {
            collision.lRef = 0;
            collision.fTime = 0.0f;
        }

        m_msgActivate = g_pEngineData->RegisterZMsg("Activate", 0, __FILE__, __LINE__);
        m_pParticleBlocks = ZUniMemory::New<ZTFixedAlloc<SParticleBlock, PARTICLES_PER_BLOCK>>();
        m_lCollisionCount = 0;
    }

    // PC dtor 0x4EF330
    ZParticleController::~ZParticleController()
    {
        if (m_pParticleBlocks != nullptr)
        {
            ZUniMemory::Delete(m_pParticleBlocks);
            m_pParticleBlocks = nullptr;
        }
    }

    const RTP::ZPropertyInfo& ZParticleController::GetProperties() const { return ZParticleController::Info; }
    uint32_t ZParticleController::GetObjectId() const { return ZParticleController::m_Id; }
    void ZParticleController::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const { id = ZParticleController::m_Id; mask = ZParticleController::m_Mask; }
    ZGEOMCLASSINFO* ZParticleController::GetOldClassInfo() const { return ZParticleController::m_OldClassInfo; }

    // PC 0x4EF?; PS2 0x22446C
    void ZParticleController::CopyData(const ZGEOM* Source)
    {
        ZGEOM::CopyData(Source);
        // The original only evaluates the derived check here; no particle state is copied.
        (void)Source->IsDerivedFrom<ZParticleController>();
    }

    // vtbl slot 118  PC 0x4EF490; PS2 0x224610
    uint32_t ZParticleController::RegisterParticleTemplate(ZREF rTemplate)
    {
        uint32_t lIndex = 0;
        for (; lIndex < MAX_NUM_TEMPLATES; ++lIndex)
        {
            const ZREF rCurrent = m_ParticleManager[lIndex].pParticleTemplate;
            if (rCurrent == 0)
                break;
            if (rCurrent == rTemplate)
                return lIndex + 1;
        }

        if (lIndex >= MAX_NUM_TEMPLATES)
            return 0;

        m_ParticleManager[lIndex].pParticleTemplate = rTemplate;
        return lIndex + 1;
    }

    // PS2 0x2244CC
    int32_t ZParticleController::ClassCommand(ZMSGID Msg, void* pData)
    {
        if (Msg == m_msgActivate)
        {
            auto* pCreate = static_cast<SCreateParticle*>(pData);
            ZASSERT(pCreate != nullptr && pCreate->lMagicNum == 0x12345678u);
            if (pCreate != nullptr)
            {
                ZVector3 vDir;
                vreset(vDir.Get());
                CreateParticle(pCreate->lTableIndex, pCreate->vPos, vDir.Get(), g_pSysInterface->m_fMainCurTime);
            }
        }
        return 0;
    }

    // vtbl slot 119  PC 0x4EFAC0
    void ZParticleController::CreateParticle(uint32_t lTableIndex, const float* pPos, const float* pDir, TIMETYPE time)
    {
        ZASSERT(lTableIndex != 0 && lTableIndex <= MAX_NUM_TEMPLATES);
        if (lTableIndex == 0 || lTableIndex > MAX_NUM_TEMPLATES)
            return;

        SParticleManager& manager = m_ParticleManager[lTableIndex - 1];
        ZParticleTemplate* pTemplate = static_cast<ZParticleTemplate*>(ZGEOM::RefToPtr(manager.pParticleTemplate));
        if (pTemplate == nullptr)
            return;

        // Reuse the template's current block unless it is full, otherwise take a
        // fresh block from the pool (PC reuses the block index in lNext).
        SParticleBlock* pBlock = nullptr;
        if (manager.lBlocks != 0)
        {
            ZASSERT(manager.lBlocks <= PARTICLES_PER_BLOCK);
            pBlock = m_pParticleBlocks->Element(manager.lBlocks);
        }

        if (pBlock == nullptr || pBlock->lCount == PARTICLES_PER_BLOCK)
        {
            if (m_pParticleBlocks->FreeCount() == 0)
                return;

            pBlock = m_pParticleBlocks->Alloc();
            pBlock->lCount = 0;
            pBlock->lNext = manager.lBlocks;
            manager.lBlocks = m_pParticleBlocks->Index(pBlock);

            // PC 0x4EFAC0: (re)initialise the block's sprite array from the template.
            pBlock->SpriteArray.lNumSprites = 0;
            pBlock->SpriteArray.pSprites = reinterpret_cast<SSpriteArrayElement*>(pBlock->SpriteElements);
            pBlock->SpriteArray.fMaxAge = pTemplate->GetMaxAge();
            pBlock->SpriteArray.fScale = pTemplate->GetScale() * (1.0f - g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * pTemplate->GetScaleVariation());
            pBlock->SpriteArray.fScaleVel = pTemplate->GetScaleVel();
            pBlock->SpriteArray.fScaleAcc = pTemplate->GetScaleAcc();
            pBlock->SpriteArray.fAngleSpeed = pTemplate->GetAngleSpeed();
            pBlock->SpriteArray.fAngleSpeedVel = pTemplate->GetAngleSpeedVel();
            pBlock->SpriteArray.fAngleSpeedAcc = pTemplate->GetAngleSpeedAcc();
            pBlock->SpriteArray.fFriction = pTemplate->GetFriction();
            pBlock->SpriteArray.fMotionStretch = pTemplate->GetMotionStretch();
            pBlock->SpriteArray.bAlignWithDir = pTemplate->GetAlignWithParticleDir();
            pBlock->SpriteArray.lColorRepeat = pTemplate->GetColorRepeat();
            pBlock->SpriteArray.vScaledGravity = pTemplate->GetScaledGravity();
            pBlock->SpriteArray.piColorTable = pTemplate->GetEffectColorTable();
            pBlock->SpriteArray.pfColorTable = pTemplate->GetColorKeys();
        }

        SSpriteArrayElementParticle* pElement = &pBlock->SpriteElements[pBlock->lCount];
        ++pBlock->lCount;
        pBlock->fNextCheckTime = 0.0f;

        const ZVector3& vScaledGravity = pTemplate->GetScaledGravity();
        const float fFriction = pTemplate->GetFriction();

        pElement->fStartScale = pTemplate->GetScale()
            * (1.0f - g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * pTemplate->GetScaleVariation());
        pElement->u8Flags = 0;
        pElement->fStartTime = static_cast<float>(time);
        pElement->fEndTime = static_cast<float>(time) + pElement->fStartScale;

        // vC2 is the per-second velocity (gravity - direction) / friction; vC1 is
        // the launch position relative to it.
        vsub(&pElement->vC2.x, vScaledGravity.Get(), pDir);
        vscalar(&pElement->vC2.x, 1.0f / fFriction);
        vsub(&pElement->vC1.x, pPos, &pElement->vC2.x);

        if (pTemplate->GetRandomStartAngle())
        {
            pElement->fStartAngle = g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * 6.2831855f;
        }
        else if (pTemplate->GetAlignWithEmitterDir())
        {
            const float fLen = std::sqrt(pDir[0] * pDir[0] + pDir[1] * pDir[1] + pDir[2] * pDir[2]);
            if (fLen == 0.0f)
            {
                pElement->fStartAngle = 0.0f;
            }
            else
            {
                float fAngle = std::acos(pDir[1] / fLen);
                if (pDir[0] / fLen <= 0.0f)
                    fAngle = -fAngle;
                pElement->fStartAngle = fAngle;
            }
        }
        else
        {
            pElement->fStartAngle = pTemplate->GetAngleStart();
        }

        pElement->u8Phase = static_cast<uint8_t>(std::rand() % 180);
        if (pTemplate->GetRandomTrajectoryEnvelope())
            pElement->u8Flags |= 4u;
        if (pTemplate->GetFake3dRotation())
            pElement->u8Flags |= 2u;

        // TODO: Finish me after the particle-collision helpers are reversed. The PC gates this
        //       block on the "particle_collision" debug int (0x7FC030): it probes a line from the
        //       spawned root point (ZCollisionBase::CalcLineColi), stores the impact in the
        //       element's root-point/flags and records the hit in m_Collisions via
        //       sub_4EEFC0/sub_4EF6E0/sub_4421E0.
        (void)g_lParticleCollision;
        (void)g_lParticleCollisionFrame;
        (void)g_lParticleCollisionCount;
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZParticleController,     // ClassName
        ZSTDOBJ,                 // BaseClass
        0x00972FC8,              // OldClassInfoAddr (ZParticleController::m_OldClassInfo)
        "ZParticleController",   // FactoryName
        0x0076C504,              // FactoryNameAddr
        nullptr,                 // FirstProperty (the class has no RTP properties)
        0x00808074,              // PropertiesAddr (ZParticleController::Info)
        0x00972F20,              // IdAddr (ZParticleController::m_Id)
        0x00972F24               // MaskAddr (ZParticleController::m_Mask)
    );
#   pragma endregion
}
