#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Animation/ZAnimVariationHandle.h>


namespace Glacier
{
    ZAnimVariationBuffer::ZAnimVariationBuffer() = default;

    void ZAnimVariationBuffer::SetBuffer(const char* pBuffer)
    {
        m_pBuffer = pBuffer;
        FindAnimListStart();
    }

    bool ZAnimVariationBuffer::IsValid() const
    {
        return m_pBuffer != nullptr;
    }

    bool ZAnimVariationBuffer::GetAnimVariation(ZAnimVariation& variation, ZAnimVariationHandle& handle)
    {
        if (IsValid() && m_AnimList && handle.IsValid() && handle.iIndex < *m_AnimList)
        {
            auto anim = m_AnimList[handle.iIndex + 1];
            if (anim == -1)
            {
                return false;
            }

            variation.SetData(&m_pBuffer[anim]);
            return true;
        }

        return false;
    }

    void ZAnimVariationBuffer::FindAnimListStart()
    {
        if (!m_pBuffer)
        {
            m_AnimList = nullptr;
            return;
        }

        const int32_t numVariations = *reinterpret_cast<const int32_t*>(m_pBuffer);
        const SFlagOffsetPair* pairs = reinterpret_cast<const SFlagOffsetPair*>(m_pBuffer + sizeof(int32_t));

        for (int32_t i = 0; i < numVariations; ++i)
        {
            while (pairs->iFlags != 0)
            {
                ++pairs;
            }

            ++pairs;
        }

        m_AnimList = const_cast<int32_t*>(reinterpret_cast<const int32_t*>(pairs) + 1);
    }

    const char* ZAnimVariationBuffer::GetBuffer() const
    {
        return m_pBuffer;
    } 
}
