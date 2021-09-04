#pragma once

#include "ZNavigator.h"

namespace Hitman::BloodMoney {
    class ZHM3InventoryMenu : public ZNavigator {
    public:
        //vftable (no changes)
        //api
        void InitInventoryList();
        void CloseItemView();
        void CloseInventoryMenu();
        void UpdateItemView();
        void UpdateItemInfo();
        void SpinCircle(bool);
        void TurnLeft();
        void TurnRight();

        //data (total size is 0x1FF8, base size is 0x98)
        int m_field98;
        int m_field9C;
        int m_field100;
        //TODO: Complete fields list
    };
}