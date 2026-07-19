#include <Glacier/Runtime/ZNonResourceClassInfo.h>
#include <utility>


namespace Glacier
{
    ZNonResourceClassInfo::ZNonResourceClassInfo(const char *szClassInfoName, int ClassInfoType, unsigned int lSize, const char *szParameters)
        : ZCLASSINFO(szClassInfoName, ClassInfoType)
        , m_lSize(lSize)
        , m_bResourceLinked(false)
        , m_lAllocatedCount(0)
        , m_lAllocatedPeakCount(0)
        , m_lRuntimeAllocatedCount(0)
        , m_lRuntimeAllocatedPeakCount(0)
    {
        (void)szParameters; // unused
    }

    ZNonResourceClassInfo::~ZNonResourceClassInfo() = default;

    uint32_t ZNonResourceClassInfo::Size() const
    {
        return m_lSize;
    }

    void ZNonResourceClassInfo::SetRuntimeAllocatedCount(uint32_t lCount)
    {
        m_lRuntimeAllocatedCount = lCount;
        if (m_lRuntimeAllocatedPeakCount < m_lRuntimeAllocatedCount)
        {
            m_lRuntimeAllocatedPeakCount = m_lRuntimeAllocatedCount;
        }
    }

    void ZNonResourceClassInfo::SetAllocatedCount(uint32_t lCount)
    {
        m_lAllocatedCount = lCount;
        if (m_lAllocatedPeakCount < m_lAllocatedCount)
        {
            m_lAllocatedPeakCount = m_lAllocatedCount;
        }
    }

    uint32_t ZNonResourceClassInfo::GetRuntimeAllocatedCount() const
    {
        return m_lRuntimeAllocatedCount;
    }

    uint32_t ZNonResourceClassInfo::GetAllocatedCount() const
    {
        return m_lAllocatedCount;
    }
}
