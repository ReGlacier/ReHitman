#pragma once

#include <Glacier/GUI/ZCHAROBJ.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/Macro.h>



namespace Glacier
{
    class ZLINEOBJ : public ZCHAROBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZLINEOBJ, 0x200038u);

        // vtbl
        ~ZLINEOBJ() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // ZCHAROBJ
        bool SetText(const char*) override;
        void Clear() override;

        // ZLINEOBJ
        virtual bool Wrap();
        virtual int GetCharWidth(const char**);
        virtual int GetStringWidth(const char* str, int length);
        virtual void SetWidth(int);
        virtual float GetWidth();

        // methods
        ZLINEOBJ(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        char* m_szText;
        int32_t m_iWidth;
        int8_t m_iNumberOfLines;
    };
    RE_VERIFY_SIZE(ZLINEOBJ, 0xAC); // PC verified
}
