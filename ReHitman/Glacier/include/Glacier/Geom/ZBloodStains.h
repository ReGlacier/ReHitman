#pragma once

#include <Glacier/Geom/ZProjectMarks.h>
#include <Glacier/ZREF.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    struct COLI;

    /**
     * @brief Projected blood-stain decals (Hitman Blood Money, zprojectmarks.cpp).
     *
     * Layout verified against the PC type database (size 0x30C) and the PS2
     * ZBloodStains constructor (PS2 0x2307FC), which wires the inherited
     * ZProjectMarks arrays to the derived buffers (m_ProjectMarksIds, m_ActivePrims,
     * m_RemovePrims, m_StartFrame, m_PreCalcPrims).
     */
    class ZBloodStains : public ZProjectMarks
    {
    public:
        // constants
        static constexpr uint32_t MAX_NR_MARKS = 32;

        // static
        static const uint32_t Default_PreCalcPrims[MAX_NR_MARKS];

        // vtbl
        ~ZBloodStains() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void ClassFrameUpdate() override;
        void CallBack(ZDecalCallBack* pDecal, uint32_t lValue) override;

        // methods
        ZBloodStains(const char* psName, ZBaseGeom* pBaseGeom);

        /**
         * @brief Tests the given position against the world collision and returns the
         *        ZREF of the geom hit by a line cast straight down (-200).
         *
         * PC 0x4F5D00; cross-checked against XBOX_KL2 0x823F5028,
         * XBOX_MiniNinjas 0x821C75C8 and iOS 0x1003652EC. All three PC call sites
         * (ZActor::ShootIntoGround 0x507A70, ZProjectileBase::ShotImpact 0x54012D,
         * ZHM3Actor::CreateBloodPoolAndCheckRetry 0x63C476) feed the result to
         * ZGEOM::RefToPtr() / ZEngineDataBase::GeomRefToPtr(), so the hit geom ref
         * is returned (0 on a miss).
         */
        ZREF MakeBloodStainColiCheck(const float* pPos, COLI& rColi) const;

        uint32_t CreateBloodStain(const COLI* pColi, uint32_t lPrim, float fMarkSize);

    protected:
        void RunMark(int lMark) override;

    public:
        // members
        uint32_t m_ProjectMarksIds[MAX_NR_MARKS];   // +0x68
        uint8_t m_ActivePrims[MAX_NR_MARKS];        // +0xE8
        uint32_t m_RemovePrims[MAX_NR_MARKS];       // +0x108
        int32_t m_StartFrame[MAX_NR_MARKS];         // +0x188
        float m_fRunFrameTime[MAX_NR_MARKS];        // +0x208
        float m_fRunMarkMultiplier;                 // +0x288
        uint32_t m_PreCalcPrims[MAX_NR_MARKS];      // +0x28C
    };
    RE_VERIFY_SIZE(ZBloodStains, 0x30C);
    RE_VERIFY_OFFSET(ZBloodStains, m_ProjectMarksIds, 0x68);
    RE_VERIFY_OFFSET(ZBloodStains, m_ActivePrims, 0xE8);
    RE_VERIFY_OFFSET(ZBloodStains, m_RemovePrims, 0x108);
    RE_VERIFY_OFFSET(ZBloodStains, m_StartFrame, 0x188);
    RE_VERIFY_OFFSET(ZBloodStains, m_fRunFrameTime, 0x208);
    RE_VERIFY_OFFSET(ZBloodStains, m_fRunMarkMultiplier, 0x288);
    RE_VERIFY_OFFSET(ZBloodStains, m_PreCalcPrims, 0x28C);
}
