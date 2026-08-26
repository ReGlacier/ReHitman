#include <Glacier/Runtime/ZNonResourceClassInfo.h>
#include <utility>


namespace Glacier
{
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
