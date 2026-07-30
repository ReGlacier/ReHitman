#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGEOM.h>
#include <cstdint>

namespace Glacier
{
    class ZSTDOBJ : public ZGEOM
    { // no changes, same size, same vftable size (maybe something overridden)
    public:
        // constants
        static constexpr uint32_t m_TypeId = 0x200002u;
        // types

        // static
#       pragma region " --- Static members --- "
        STATIC_CLASS_VAR(ZSTDOBJ, const char*, FactoryName);
        STATIC_CLASS_VAR(ZSTDOBJ, RTP::ZPropertyInfo, Info);
        STATIC_CLASS_VAR(ZSTDOBJ, ZGEOMCLASSINFO*, m_OldClassInfo);
        DECLARE_ID_AND_MASK(ZSTDOBJ);
#       pragma endregion

        // vtbl
        // methods
        ZSTDOBJ(const char* psName, ZBaseGeom* pBaseGeom);
        ~ZSTDOBJ() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;
        void Select(bool OnOff) override;
        bool RequestCustomDraw() override;
        eGlobalTreeType GetBoundTreeType() const override;
        void DispBound(bool OnOff) override;
        void ClassInit() override;
        bool ChkLineColi(COLI* pColi, bool bWantInvisible) override;
        bool CheckPointInside(ZVector3& pPoint, float fDotDist) override;
        bool CheckBoxInside(const ZMat3x3& mMat, const ZVector3& vPos, const float* s0) override;

        static uint32_t GetClassId();        

#       pragma region " --- RTTI Methods --- "
        void SetInvisible(const bool&);
        void GetInvisible(bool&);
#       pragma endregion
    };
    RE_VERIFY_SIZE(ZSTDOBJ, 0x10); // Verified
}
