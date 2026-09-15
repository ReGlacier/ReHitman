#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    struct ZAllocMany : public ZGROUP
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZAllocMany, 0x1000CAu);

        // vtbl
        ~ZAllocMany() override;

        // ZSerializable
        void PostSave(ISerializerStream& stream) override;
        bool PostLoad(ISerializerStream& stream) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void Activate(bool bActive) override;
        void ClassInit2() override;
        void PostClassInit() override;
        ZGEOM* Duplicate(ZGROUP* DestGroup, const char* DupName, bool Recursive) override;
        ZGEOM* DuplicateInit(ZGROUP* DestGroup, const ZMat3x3* pMat, const ZVector3* pPos, char const* DupName, bool Recursive) override;
        void CopyData(const ZGEOM* Source) override;

        // methods
        ZAllocMany(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        int16_t m_iNumObjects;        // +0x4C
        int16_t m_iNumFree;           // +0x4E
        int16_t m_iFreeIndex;         // +0x50
        bool m_bNeedClean;            // +0x52
        RE_ADD_PADDING(1);            // +0x53
        ZAllocMany* m_pOriginal;      // +0x54
        uint16_t m_msgActivate;       // +0x58
        bool m_bClone;                // +0x5A
        RE_ADD_PADDING(1);            // +0x5B
        uint32_t m_rCloneGroup;       // +0x5C
        bool m_bInitClones;           // +0x60
        RE_ADD_PADDING(3);            // +0x61-0x63
        ZAllocMany** m_pFreeObjects;  // +0x64
    };
    RE_VERIFY_SIZE(ZAllocMany, 0x68); // Verified
    RE_VERIFY_OFFSET(ZAllocMany, m_iNumObjects, 0x4C);
    RE_VERIFY_OFFSET(ZAllocMany, m_iNumFree, 0x4E);
    RE_VERIFY_OFFSET(ZAllocMany, m_iFreeIndex, 0x50);
    RE_VERIFY_OFFSET(ZAllocMany, m_bNeedClean, 0x52);
    RE_VERIFY_OFFSET(ZAllocMany, m_pOriginal, 0x54);
    RE_VERIFY_OFFSET(ZAllocMany, m_msgActivate, 0x58);
    RE_VERIFY_OFFSET(ZAllocMany, m_bClone, 0x5A);
    RE_VERIFY_OFFSET(ZAllocMany, m_rCloneGroup, 0x5C);
    RE_VERIFY_OFFSET(ZAllocMany, m_bInitClones, 0x60);
    RE_VERIFY_OFFSET(ZAllocMany, m_pFreeObjects, 0x64);
}
