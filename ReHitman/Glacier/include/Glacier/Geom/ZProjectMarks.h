#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZBaseGeom;
    struct COLI;
    struct ZDecalCallBack;

    /**
     * @brief Base class for "projected mark" geoms (blood stains, splatters, bullet
     *        marks, foot prints, blood trails, ...).
     *
     * Layout verified against the PC type database (size 0x68, first own field at
     * +0x14) and the PS2 ZProjectMarks / ZBloodStains constructors
     * (PS2 0x22E5F4 / 0x2307FC), which confirm every field offset below.
     */
    class ZProjectMarks : public ZSTDOBJ
    {
    public:
        // vtbl
        ~ZProjectMarks() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void ClassFrameUpdate() override;

        // ZProjectMarks
        virtual void SaveCleanup(bool bSaving);
        virtual void CallBack(ZDecalCallBack* pDecal, uint32_t lValue);
        virtual void RemoveAllMarks();

        // methods
        ZProjectMarks(const char* psName, ZBaseGeom* pBaseGeom);

    protected:
        void UpdateActiveMarks();
        uint32_t AddActive();
        virtual void FadeMark(uint32_t lMark, float fFade);
        uint32_t GetRemovePrimIndex();
        void RemoveActive(uint32_t lMark);
        uint32_t CreateMark(const COLI* pColi, uint32_t lPrim, float f0, float f1, float f2);
        uint32_t CreateMark(const COLI* pColi, uint32_t lPrim, float f0, float f1, const float* s0, float f2);
        virtual void RemoveMark(uint32_t lMark);
        virtual void RunMark(int lMark);
        bool IsTimeForSeeableDecal();
        void PlotStatus();

    public:
        // members
        RE_ADD_PADDING(4);                  // +0x10 unreversed member
        uint32_t m_lMaxPersistentMarks;     // +0x14
        uint32_t m_lMaxNrMarks;             // +0x18
        uint32_t m_lMaxNrMarksTypes;        // +0x1C
        int32_t m_lFrameCount;              // +0x20
        float m_fFade;                      // +0x24
        float m_fRemove;                    // +0x28
        bool m_bMakeVisible;                // +0x2C
        RE_ADD_PADDING(3);                  // +0x2D
        float m_fTimeBetweenVisibleMarks;   // +0x30
        float m_fMinDistBetweenSeeables;    // +0x34
        uint32_t m_lNrRemove;               // +0x38
        uint32_t m_lNrActive;               // +0x3C
        uint32_t m_lWriteCount;             // +0x40
        uint32_t m_lReadCount;              // +0x44
        uint32_t* m_pProjectMarksIds;       // +0x48
        uint32_t m_lArrayIndex;             // +0x4C
        uint32_t* m_PrimsArray;             // +0x50
        uint8_t* m_pActivePrims;            // +0x54
        uint32_t* m_pRemovePrims;           // +0x58
        int32_t* m_pStartFrame;             // +0x5C
        bool m_bStoreUV;                    // +0x60
        RE_ADD_PADDING(3);                  // +0x61
        TIMETYPE m_ttLastSeeableDecal;      // +0x64
    };
    RE_VERIFY_SIZE(ZProjectMarks, 0x68);
    RE_VERIFY_OFFSET(ZProjectMarks, m_lMaxPersistentMarks, 0x14);
    RE_VERIFY_OFFSET(ZProjectMarks, m_lMaxNrMarks, 0x18);
    RE_VERIFY_OFFSET(ZProjectMarks, m_lMaxNrMarksTypes, 0x1C);
    RE_VERIFY_OFFSET(ZProjectMarks, m_lFrameCount, 0x20);
    RE_VERIFY_OFFSET(ZProjectMarks, m_fFade, 0x24);
    RE_VERIFY_OFFSET(ZProjectMarks, m_fRemove, 0x28);
    RE_VERIFY_OFFSET(ZProjectMarks, m_bMakeVisible, 0x2C);
    RE_VERIFY_OFFSET(ZProjectMarks, m_fTimeBetweenVisibleMarks, 0x30);
    RE_VERIFY_OFFSET(ZProjectMarks, m_fMinDistBetweenSeeables, 0x34);
    RE_VERIFY_OFFSET(ZProjectMarks, m_lNrRemove, 0x38);
    RE_VERIFY_OFFSET(ZProjectMarks, m_lNrActive, 0x3C);
    RE_VERIFY_OFFSET(ZProjectMarks, m_lWriteCount, 0x40);
    RE_VERIFY_OFFSET(ZProjectMarks, m_lReadCount, 0x44);
    RE_VERIFY_OFFSET(ZProjectMarks, m_pProjectMarksIds, 0x48);
    RE_VERIFY_OFFSET(ZProjectMarks, m_lArrayIndex, 0x4C);
    RE_VERIFY_OFFSET(ZProjectMarks, m_PrimsArray, 0x50);
    RE_VERIFY_OFFSET(ZProjectMarks, m_pActivePrims, 0x54);
    RE_VERIFY_OFFSET(ZProjectMarks, m_pRemovePrims, 0x58);
    RE_VERIFY_OFFSET(ZProjectMarks, m_pStartFrame, 0x5C);
    RE_VERIFY_OFFSET(ZProjectMarks, m_bStoreUV, 0x60);
    RE_VERIFY_OFFSET(ZProjectMarks, m_ttLastSeeableDecal, 0x64);
}
