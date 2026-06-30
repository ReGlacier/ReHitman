#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>

namespace Glacier
{
    class STRREFTAB : public REFTAB
    {
    public:
        // VFTable
        virtual void* FindStr(const char*);
        virtual void AddStr(const char*);
        virtual void AddAlways(const char*);
        virtual void RemoveStr(const char*);
        virtual bool Exists(const char*);
        virtual void Sort();

        // Data
        bool m_bCaseSensitive; //0x001C
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(STRREFTAB, 0x20);
    RE_VERIFY_OFFSET(STRREFTAB, m_bCaseSensitive, 0x1C);
}