#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ZHandle.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZWinEvents.h>

#include <BloodMoney/Game/SMapGroup.h>
#include <BloodMoney/Game/SIconBase.h>
#include <BloodMoney/Game/UI/ZWINDOW.h>
#include <BloodMoney/Game/UI/CWinEvent.h>

namespace Hitman::BloodMoney
{
    struct SMapText
    {
        int id; //0x0000
        bool field_04;   //0x0004
        bool field_05;   //0x0005
        bool field_06;   //0x0006
        bool field_07;   //0x0007
        char str[0x5C];  //0x0008
        int  field_64;   //0x0064
        int  field_68;
        int  field_6C;
        int  field_70;
        int  field_74;
        int  field_78;
        int  field_7C;
        int  field_80;
        int  field_84;
        int  field_88;
        int  field_8C;
        int  field_90;
        int  field_94;
    }; //Size: 0x0098 (see CIngameMap::AddText at 00663E20 for details)

    struct SIconInstance
    {
        int32_t ref;         // SRef id (object id)
        bool field_04;       // must be true if you want to replace icon
        char field_5;        // ?
        char field_6;        // ?
        char field_7;        // ?
        int32_t eIconType;   // icon type (enum)
    };

    class CIngameMap : public CWinEvent<ZWINDOW>
    {
    public:
        //vftable
        virtual void AddMapGroup(uint16_t,char const*,uint16_t);
        virtual void OpenMap(void);
        virtual void CloseMap(void);
        virtual void AddIcon(SIconInstance*,bool);
        virtual SIconInstance* GetIcon(uint16_t);
        virtual void RemoveIcon(SIconInstance*);
        virtual SMapText* AddText(SMapText const*);
        virtual void RemoveText(SMapText const*);
        virtual void NotifyUpdate(uint16_t);

        //data (total size is 0x2C4, CWinEvent<ZWINDOW> : ZEventBase size is 0x30)
        Glacier::ZHandle m_mapLegendAction;
        Glacier::ZHandle m_mapSelectAction;
        Glacier::ZHandle m_mapCameraLeftAction;
        Glacier::ZHandle m_mapCameraRightAction;
        Glacier::ZHandle m_mapCameraUpAction;
        Glacier::ZHandle m_mapCameraDownAction;
        Glacier::ZHandle m_mapMoveUpAction;
        Glacier::ZHandle m_mapMoveDownAction;
        Glacier::ZHandle m_mapPrevAction;
        Glacier::ZHandle m_mapNextAction;
        Glacier::ZHandle m_mapMenuAction;
        Glacier::ZHandle m_mapAction;
        int field_00C0;
        int field_00C4;
        int field_00C8;
        int field_00CC;
        int field_00D0;
        int field_00D4;
        int field_00D8;
        int field_00DC;
        int field_00E0;
        int field_00E4;
        int field_00E8;
        int field_00EC;
        int field_00F0;
        int field_00F4;
        Glacier::REFTAB32 m_reftab32_00F8;
        int field_01A4;
        SMapGroup* m_legendMapGroup;
        int field_01AC;
        Glacier::REFTAB m_mapGroups; //REFTAB of SMapGroup
        int field_01CC;
        int field_01D0;
        Glacier::REFTAB m_iconsRefTab;
        Glacier::REFTAB m_textsRefTab;
        int field_020C;
        int field_0210;
        int field_0214;
        int field_0218;
        int field_021C;
        int field_0220;
        int field_0224;
        int field_0228;
        int field_022C;
        int field_0230;
        int field_0234;
        int field_0238;
        int field_023C;
        int field_0240;
        int field_0244;
        int field_0248;
        int field_024C;
        int field_0250;
        int field_0254;
        int field_0258;
        int field_025C;
        int field_0260;
        int field_0264;
        int field_0268;
        int field_026C;
        int field_0270;
        int field_0274;
        int field_0278;
        int field_027C;
        int field_0280;
        int field_0284;
        int field_0288;
        int field_028C;
        int field_0290;
        int field_0294;
        char m_isMapListOpened;
        char field_299;
        char field_29A;
        char field_29B;
        char field_029C;
        char field_29D;
        char m_mapsComboShowing;
        char field_29F;
        char field_02A0;
        char field_2A1;
        char field_2A2;
        char field_2A3;
        int field_02A4;
        int field_02A8;
        SMapGroup *m_currentMapGroup;
        int field_02B0;
        int field_02B4;
        int field_02B8;
        int field_02BC;
        int field_02C0;
    };
}