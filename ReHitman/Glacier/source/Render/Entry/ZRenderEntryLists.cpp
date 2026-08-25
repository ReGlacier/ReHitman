#include <Glacier/Render/Entry/ZRenderEntryLists.h>
#include <Glacier/Render/Entry/ZRenderEntryGeom.h>


namespace Glacier
{
    ZRenderEntryLists::ZRenderEntryLists()
    {
        m_Lists[LT_BONES] = {};
        m_Lists[LT_LIGHT] = {};
        m_Lists[LT_DRAWUPDATE] = {};
        m_Lists[LT_ENVIRONMENT] = {};
        m_Lists[LT_STATICSHADOW] = {};
        m_pViewList = nullptr;
        m_pParent = nullptr;
        CalculateListId();
    }

    ZRenderEntryLists::~ZRenderEntryLists()
    {
        if (m_pParent)
        {
            m_pParent->m_pViewList = nullptr;
        }

        const auto lCtlMask = 1 << static_cast<uint32_t>(m_lListId);
        for (int i = LT_LISTSIZE - 1; i != -1; --i)
        {
            auto* pCurrent = GetList(static_cast<LISTTYPES>(i));
            for (int j = 0; j < pCurrent->Count(); ++j)
            {
                auto* pEntry = *pCurrent->Get(j);
                ZASSERT(pEntry);

                pEntry->m_lGeomListsControl &= ~lCtlMask;
            }
        }

        ZRenderEntryLists::m_lActiveListIds &= ~lCtlMask;
    }

    ZStackArray<ZRenderEntryLists::MAX_ENTRIES_IN_LIST, ZRenderEntryGeom*>* ZRenderEntryLists::GetList(LISTTYPES lType)
    {
        return &m_Lists[lType];
    }

    void ZRenderEntryLists::Add(ZRenderEntryGeom* pEntry)
    {
        if (!pEntry->m_lGeomListsControl)
        {
            return;
        }

        if (m_pViewList)
            m_pViewList->Add(pEntry);

        if (((1 << m_lListId) & pEntry->m_lEntryListsMask) == 0)
        {
            // Assign bit of our list (we are member of this render entry list)
            pEntry->m_lEntryListsMask |= (1 << m_lListId);

            uint32_t lListIndex = 0;
            auto* pCurrentList = &m_Lists[0];
            uint32_t lControl = pEntry->m_lGeomListsControl;
            do
            {
                if ((lControl & 1) != 0)
                {
                    ZASSERT(lListIndex < LISTTYPES::LT_LISTSIZE);
                    pCurrentList->Add(&pEntry);
                }

                lControl >>= 1;
                ++pCurrentList;
            }
            while (lControl);
        }
    }

    void ZRenderEntryLists::CalculateListId()
    {
        ZASSERT(ZRenderEntryLists::m_lActiveListIds < 0xFFu);

        uint8_t lFreeBit = 0;
        while (((1 << lFreeBit) & ZRenderEntryLists::m_lActiveListIds) != 0)
        {
            if (++lFreeBit == 8)
                return;
        }

        ZRenderEntryLists::m_lActiveListIds |= 1 << lFreeBit;
        m_lListId = ZRenderEntryLists::m_lActiveListIds;
    }

    STATIC_CLASS_VAR_IMPL(ZRenderEntryLists, uint8_t, m_lActiveListIds, 0x008EC160, 0);
}
