#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZOMNILIGHT.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZGateLightOmni : public ZOMNILIGHT
    {
    public:
        // types
        enum EType
        {
            CLONES = 0,
            AIMATGATES = 1
        };

        enum EFilterType
        {
            EXCLUDE = 0,
            INCLUDE = 1
        };

        // RTTI
        DECLARE_GEOM_CLASS(ZGateLightOmni, 0x8000F5u);

        // vtbl
        ~ZGateLightOmni() override;

        // ZSerializable

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void Activate(bool bActive) override;
        void SetMat(const Glacier::ZMat3x3& mMat) override;
        void SetPos(const Glacier::ZVector3& vPos) override;
        void SetMatPos(const Glacier::ZMat3x3& mat, const Glacier::Vector3& pos) override;
        void Display(bool OnOff) override;
        void ClassInit2() override;
        void CopyData(const ZGEOM* Source) override;

        // ZLIGHT
        void Enable() override;
        void Disable() override;
        void SetMultiplier(float fValue) override;

        // methods
        ZGateLightOmni(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        EType m_lType;
        REFTAB m_RoomsFilter;
        EFilterType m_lFilterType;
        REFTAB m_Slaves; // Original naming :pepe:
        REFTAB m_Gates;
    };
    RE_VERIFY_SIZE(ZGateLightOmni, 0x7C); // Verified PC alloc
}
