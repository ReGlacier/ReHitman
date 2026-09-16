#pragma once

#include <Glacier/ZSTL/zstring.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <Glacier/GUI/eZWUserEvents.h>


namespace Glacier
{
    class ZWINOBJ;

    // Interface consumed by ZGUIList. PC vtable order (0x7A4CEC) and the byte offsets
    // dispatched by ZGUIList::Setup / SetupTextOnButtons (0x04..0x4C) match verbatim.
    class ZListDataInterface
    {
    public:
        // vtbl
        virtual ~ZListDataInterface() = default;
        virtual void GetCellText(int iIndex, int iColumn, zstring& rText) = 0;
        virtual void Click(int iIndex, eZWUserEvents eEvent);
        virtual void Activate(int iIndex);
        virtual void SelectItem(int iIndex);
        virtual bool GetChecked(int iIndex, ZStaticVector<ZWINOBJ*, 8>* pChecked, ZStaticVector<ZWINOBJ*, 8>* pUnchecked);
        virtual void FocusChanged(int iIndex);
        virtual void Begin() = 0;
        virtual int GetNumOfItems() = 0;
        virtual void SetWindow(zstring sWindow);
        virtual void Close();
        virtual int GetMaxNumOfButtons();
        virtual int GetFocus();
        virtual int GetSelectedItem();
        virtual void Cancel();
        virtual void Update();
        virtual bool IgnoreLineInGUI(int iIndex);
        virtual const char* GetColorSet(int iIndex);
        virtual const char* GetButtonGraphic(int iIndex);
        virtual const char* GetButtonGraphicMultiple(int iIndex, int iColumn);
    };

    inline void ZListDataInterface::Click(int iIndex, eZWUserEvents eEvent)
    {
        if (eEvent == eZW_SELECT)
            Activate(iIndex);
    }

    inline void ZListDataInterface::Activate(int)
    {
    }

    inline void ZListDataInterface::SelectItem(int)
    {
    }

    inline bool ZListDataInterface::GetChecked(int, ZStaticVector<ZWINOBJ*, 8>*, ZStaticVector<ZWINOBJ*, 8>*)
    {
        return false;
    }

    inline void ZListDataInterface::FocusChanged(int)
    {
    }

    inline void ZListDataInterface::SetWindow(zstring)
    {
    }

    inline void ZListDataInterface::Close()
    {
    }

    inline int ZListDataInterface::GetMaxNumOfButtons()
    {
        return 4;
    }

    inline int ZListDataInterface::GetFocus()
    {
        return 0;
    }

    inline int ZListDataInterface::GetSelectedItem()
    {
        return -1;
    }

    inline void ZListDataInterface::Cancel()
    {
    }

    inline void ZListDataInterface::Update()
    {
    }

    inline bool ZListDataInterface::IgnoreLineInGUI(int)
    {
        return false;
    }

    inline const char* ZListDataInterface::GetColorSet(int)
    {
        return nullptr;
    }

    inline const char* ZListDataInterface::GetButtonGraphic(int)
    {
        return nullptr;
    }

    inline const char* ZListDataInterface::GetButtonGraphicMultiple(int iIndex, int iColumn)
    {
        return iColumn ? nullptr : GetButtonGraphic(iIndex);
    }

    // Interface consumed by ZGUITable. PC layout verified against the concrete
    // ZStatLevelRatingTable vtable (0x7A88C4): 0x00..0x18.
    class ITableDataInterface
    {
    public:
        // vtbl
        virtual zstring GetColumnTitle(int iColumn) = 0;
        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual zstring GetCellText(int iRow, int iColumn) = 0;
        virtual void SelectRow(int iRow) = 0;
        virtual void Setup() = 0;
        virtual void GetSize(int& iNumOfRows, int& iNumOfColumns) = 0;
    };
}
