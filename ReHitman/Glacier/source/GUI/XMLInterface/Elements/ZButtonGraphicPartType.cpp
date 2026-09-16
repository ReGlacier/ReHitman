#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphicPartType.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>


namespace Glacier
{
    ZButtonGraphicPartType::ZButtonGraphicPartType()
    {
        m_iType = 0;
    }

    void ZButtonGraphicPartType::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZGUIBase::readParams(ppParams, pElems);

        ReadType(m_iType, ppParams);
    }
}
