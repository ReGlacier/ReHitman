#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>

namespace Glacier
{
    class STRREFTAB : public REFTAB
    {
    public:
        // methods
        STRREFTAB(int pPoolSize, int pUserData);

        // vtbl - override
        ~STRREFTAB() override;
        void Clear() override;
        void ClearThis() override;
        void RunDelRef(RefRun *) override;
        // vtbl - new methods
        virtual uint32_t* FindStr(const char*);
        virtual uint32_t* AddStr(const char*);
        virtual uint32_t* AddAlways(const char*);
        virtual void RemoveStr(const char*);
        virtual bool Exists(const char*);
        virtual void Sort();

        // custom iterators
        REFTAB::Iterator<const char*> begin() { return REFTAB::Iterator<const char*>(this, false); }
        REFTAB::Iterator<const char*> end()   { return REFTAB::Iterator<const char*>(this, true); }

        // Data
        bool m_bCaseSensitive{false}; //0x001C
        RE_ADD_PADDING(3);

    private:
        bool Equals(const char* pString1, const char* pString2) const;
    };
    RE_VERIFY_SIZE(STRREFTAB, 0x20);
    RE_VERIFY_OFFSET(STRREFTAB, m_bCaseSensitive, 0x1C);
}