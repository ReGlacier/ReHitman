#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    class IOptionsInterface
    {
    public:
        enum EOPTION_TYPE
        {
            EMainIni = 0,
            EProfile = 1,
        };

        // vtbl
        virtual int32_t GetOptionValue(const char* pszName, int32_t iDefault) = 0;
        virtual void SetOptionValue(const char* pszName, int32_t iValue, int32_t iUnused) = 0;
        virtual void GetOptionTextValue(const char* pszName, zstring& rText) = 0;
        virtual void ApplyOptions() = 0;
        virtual EOPTION_TYPE GetOptionType(const char* pszName) = 0;
        virtual bool IsOptionEnabled(const char* pszName, int32_t iDefault) = 0;
        virtual bool UseDynamicBounds(const char* pszName, int32_t& rLower, int32_t& rUpper) = 0;
        virtual void SaveOptions(int32_t* pValues, int32_t iCount, int32_t iUnused) = 0;
        virtual void LoadOptions(const int32_t* pValues, int32_t iCount, int32_t iUnused) = 0;
        virtual int32_t GetSize() = 0;
        virtual void SetFeedback(bool bFeedback) = 0;
        virtual void ReadGamerDefaults() = 0;
    };
    RE_VERIFY_SIZE(IOptionsInterface, 0x4);
}
