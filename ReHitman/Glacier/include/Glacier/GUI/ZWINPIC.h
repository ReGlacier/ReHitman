#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZWINPIC : public ZWINOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZWINPIC, 0x200046u);

        // vtbl
        ~ZWINPIC() override;
        // ZSerializable
        bool PostLoad(ISerializerStream& stream) override;
        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;
        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        // ZWINOBJ
        void LoadSaveGeometry(ISerializerStream& stream, bool bSaving) override;
        // ZWINPIC
        virtual void SetUV(float u0, float v0, float u1, float v1);
        virtual void Mirror(bool bMirror);

        // methods
        ZWINPIC(const char* psName, ZBaseGeom* pBaseGeom);
        void ConvertIndicesToPtrs();
        void SetPicResourcePrim(const uint32_t& pPrimID);
        void GetPicResourcePrim(uint32_t& primID);

        // data
        uint32_t m_iOriginalSizeX;
        uint32_t m_iOriginalSizeY;
        uint32_t m_dwModifications;
        ZVector4 m_v4UVModified;
    };
    RE_VERIFY_SIZE(ZWINPIC, 0xA4);
}
