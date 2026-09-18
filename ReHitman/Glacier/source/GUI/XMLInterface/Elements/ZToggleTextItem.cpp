#include <Glacier/GUI/XMLInterface/Elements/ZToggleTextItem.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>


namespace Glacier
{
    ZToggleTextItem::ZToggleTextItem()
    {
    }

    ZToggleTextItem::~ZToggleTextItem()
    {
    }

    void ZToggleTextItem::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZGUIBase::readParams(ppParams, pElems);

        GuiOption::readString(m_sText, "Text", ppParams);
    }

    const char* ZToggleTextItem::GetText() const
    {
        return m_sText.c_str();
    }
}
