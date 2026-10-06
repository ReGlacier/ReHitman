#include <Glacier/Animation/ZAnimVariationHandle.h>


namespace Glacier
{
    ZAnimVariationHandle::ZAnimVariationHandle() = default;

    ZAnimVariationHandle::ZAnimVariationHandle(const ZAnimVariationHandle& copy)
        : iIndex{copy.iIndex}
    {
    }

    ZAnimVariationHandle& ZAnimVariationHandle::operator=(const ZAnimVariationHandle& copy)
    {
        iIndex = copy.iIndex;
        return *this;
    }

    bool ZAnimVariationHandle::IsValid() const
    {
        return iIndex >= 0;
    }
}