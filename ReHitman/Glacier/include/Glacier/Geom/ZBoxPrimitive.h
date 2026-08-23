#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ReGlacier.h>

namespace Glacier
{
    class ZBoxPrimitive : public ZSTDOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZBoxPrimitive, 0x2000E1u);

        // vtbl
        ~ZBoxPrimitive() override;

        // ZSerializable
        bool PostLoad(ISerializerStream& stream) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;
        bool CheckPointInside(ZVector3& pPoint, float fDotDist) override;
        bool CheckBoxInside(const ZMat3x3& mMat, const ZVector3& vPos, const float* s0) override;
        void CopyData(const ZGEOM* Source) override;

        // ZBoxPrimitive
        virtual void CalcPositionInBox(ZVector3& pos);
        virtual bool GetClosestPosDirInBox(const ZVector3& vTarget, float edgeDist, ZVector3& pos, ZVector3& dir);
        virtual void SetScale(float x, float y, float z);
        virtual void GetScale(ZVector3& vScale);

        // methods
        ZBoxPrimitive(const char* psName, ZBaseGeom* pBaseGeom);

        // RTTI methods
        void GetScaleX(float& fScaleX);
        void SetScaleX(const float& fScaleX);
        void GetScaleY(float& fScaleY);
        void SetScaleY(const float& fScaleY);
        void GetScaleZ(float& fScaleZ);
        void SetScaleZ(const float& fScaleZ);

        // members
        Glacier::ZVector3 m_vScale;
    };
    RE_VERIFY_SIZE(ZBoxPrimitive, 0x1C); // Verified
    RE_VERIFY_OFFSET(ZBoxPrimitive, m_vScale, 0x10);
}
