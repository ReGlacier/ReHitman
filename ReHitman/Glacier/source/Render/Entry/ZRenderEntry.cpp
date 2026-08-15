#include <Glacier/Render/Entry/ZRenderEntry.h>


namespace Glacier
{
    void ZRenderEntry::SetObjectToWorldMatrix(const ZMatrix& mMatrix)
    {
        // PC (0x00476800): copies the matrix dword-wise and raises RE_HASMOVED only if something actually changed
        auto* pDst = reinterpret_cast<uint32_t*>(&m_ObjectToWorldMatrix);
        const auto* pSrc = reinterpret_cast<const uint32_t*>(&mMatrix);

        bool bChanged = false;
        for (size_t i = 0; i < sizeof(ZMatrix) / sizeof(uint32_t); ++i)
        {
            if (pDst[i] != pSrc[i])
            {
                pDst[i] = pSrc[i];
                bChanged = true;
            }
        }

        if (bChanged)
        {
            m_lControl |= RE_HASMOVED;
        }
    }
}