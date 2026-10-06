#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/Render/Config.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderEntryLists
    {
    public:
        // globals
        STATIC_CLASS_VAR(ZRenderEntryLists, uint8_t, m_lActiveListIds);

        // constants
        static constexpr int MAX_ENTRIES_IN_LIST = ELEMENTS_IN_RENDER_ENTRY_LIST_COUNT;

        // types
        enum LISTTYPES : int32_t
        {
            LT_BONES        = 0x0,
            LT_LIGHT        = 0x1,
            LT_DRAWUPDATE   = 0x2,
            LT_ENVIRONMENT  = 0x3,
            LT_STATICSHADOW = 0x4,
            LT_LISTSIZE     = 0x5,
        };

        // methods
        ZRenderEntryLists();
        ~ZRenderEntryLists();
        ZStackArray<MAX_ENTRIES_IN_LIST, ZRenderEntryGeom*>* GetList(LISTTYPES lType);
        void Add(ZRenderEntryGeom* pEntry);
        void CalculateListId();

        // members
        uint8_t m_lListId;
        RE_ADD_PADDING(3);
        ZRenderEntryLists* m_pViewList;
        ZRenderEntryLists* m_pParent;
        ZStackArray<MAX_ENTRIES_IN_LIST, ZRenderEntryGeom*> m_Lists[LISTTYPES::LT_LISTSIZE];
    };
    RE_VERIFY_OFFSET(ZRenderEntryLists, m_Lists, 0xC); // Verified by ZRenderEntryLists::GetList
}
