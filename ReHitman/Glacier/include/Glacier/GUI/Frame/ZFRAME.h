#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class REFTAB32;

    class ZFRAME : public ZWINOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZFRAME, 0x200045u);

        // vtbl
        ~ZFRAME() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void PostClassInit() override;
        void CopyData(const ZGEOM* pSource) override;

        virtual void SetSize(int lWidth, int lHeight);
        virtual void SetOuterSize(int lWidth, int lHeight);
        virtual void CreateGeometry();

        // methods
        ZFRAME(const char* psName, ZBaseGeom* pBaseGeom);

        void GetPieces(REFTAB32& pieces);
        void SetPieces(const REFTAB32& pieces);

        // members
        uint32_t m_aPieces[9];
        uint32_t m_lTextSize;
        int32_t m_lWidth;
        int32_t m_lHeight;
    };
    RE_VERIFY_SIZE(ZFRAME, 0xB8); // Verified PC alloc
    RE_VERIFY_OFFSET(ZFRAME, m_aPieces, 0x88);
    RE_VERIFY_OFFSET(ZFRAME, m_lTextSize, 0xAC);
    RE_VERIFY_OFFSET(ZFRAME, m_lWidth, 0xB0);
    RE_VERIFY_OFFSET(ZFRAME, m_lHeight, 0xB4);
}
