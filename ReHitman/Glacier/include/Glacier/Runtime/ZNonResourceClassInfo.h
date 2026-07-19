#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/ZCLASSINFO.h>
#include <cstdint>


namespace Glacier
{
    struct ZNonResourceClassInfo : public ZCLASSINFO
    {
        // methods
        ZNonResourceClassInfo(const char *szClassInfoName, int ClassInfoType, uint32_t lSize, const char *szParameters);
        ~ZNonResourceClassInfo();

        uint32_t Size() const;
        void SetRuntimeAllocatedCount(uint32_t lCount);
        void SetAllocatedCount(uint32_t lCount);
        uint32_t GetRuntimeAllocatedCount() const;
        uint32_t GetAllocatedCount() const;

        // members
        uint32_t m_lSize;
        bool m_bResourceLinked;
        uint32_t m_lAllocatedCount;
        uint32_t m_lAllocatedPeakCount;
        uint32_t m_lRuntimeAllocatedCount;
        uint32_t m_lRuntimeAllocatedPeakCount;
    };
    RE_VERIFY_SIZE(ZNonResourceClassInfo, 0x30);
}