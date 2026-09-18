#include <Glacier/Audio/ZAudioType.h>

namespace Glacier
{
    ZAudioTypeBase::ZPackedBase::ZPackedBase()
        : m_Type(ZAudioTypes::Base)
        , m_Filler1(0)
        , m_ResourceNameOffset(0)
        , m_lControllerCount(0)
        , m_lControllerOffset(0)
    {
    }

    ZAudioTypeBase::~ZAudioTypeBase() = default;

    ZAudioTypes::EType ZAudioTypeBase::GetType() const
    {
        return ZAudioTypes::Base;
    }

    ZAudioTypeBase::ZPackedBase& ZAudioTypeBase::GetPackedTemplate()
    {
        static ZPackedBase packed;
        return packed;
    }

    ZAudioTypeBase::ZPackedBase* ZAudioTypeBase::GetNewPackedTemplate()
    {
        return ZUniMemory::New<ZPackedBase>();
    }

    size_t ZAudioTypeBase::GetPackedTemplateSize() const
    {
        return sizeof(ZPackedBase);
    }

    bool ZAudioTypeBase::IsPackingUnique() const
    {
        return true;
    }
}
