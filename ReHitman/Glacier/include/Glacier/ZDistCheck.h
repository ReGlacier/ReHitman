#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>

#include <cstdint>

namespace Glacier
{
    struct IInputSerializerStream;
    struct IOutputSerializerStream;

    // Distance/priority bucketing table shared by ZActionController (same source
    // TU as action.cpp in the original engine, "Z:\code\engine\eventsbase\action.cpp").
    //
    // It keeps a flat table of object refs (m_pArray) split into up to 8 distance
    // sections (bounds in m_aLimits, per-section priority in m_fPrio), plus a single
    // "other object" ref (m_rOtherObject) whose distance is used as the reference frame.
    class ZDistCheck
    {
    public:
        // vtbl
        virtual ~ZDistCheck();
        virtual void LoadObject(IInputSerializerStream& stream);
        virtual void SaveObject(IOutputSerializerStream& stream);

        // methods
        ZDistCheck();

        void Init();
        void Update();
        int32_t Add(unsigned int lRef);
        void UpdateSingleObject(unsigned int lRef);
        void Remove(int lIndex);
        int32_t Find(unsigned int lRef);
        void SetOtherObj(unsigned int lRef);
        ZREF GetOtherObj();
        ZREF Get(int lIndex);
        int32_t Count(int lSectionNr);
        bool Exists(unsigned int lRef);
        void Migrate(ZDistCheck* pOther);

        // internal bucket helpers
        void UpdateSection(int lSectionNr, float* pValue);
        int32_t Promote(int lIndex);
        int32_t Demote(int lIndex);
        int32_t GetSectionNr(int lIndex);
        void Swap(int lIndexA, int lIndexB);
        int32_t GetSectionDist(int lSectionNr);
        void Verify();

        // members (total size 0x1048; vptr occupies +0x00)
        ZREF m_rOtherObject;  // +0x04
        ZREF m_pArray[1024];  // +0x08
        int32_t m_aLimits[8]; // +0x1008
        float m_fPrio[8];     // +0x1028
    };
    RE_VERIFY_SIZE(ZDistCheck, 0x1048); // PC verified
}
