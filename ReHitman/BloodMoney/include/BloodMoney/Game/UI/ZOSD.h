#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/ZSDOwner.h>

namespace Hitman::BloodMoney
{
    class ZLINEOBJ;
	class ZCheatMenu;

    class ZOSD : public Glacier::ZSTDOBJ
    {
    public:
        //vftable (no custom methods here)
        // custom api
        /**
         * @brief Show notification message
         * @param message
         * @param shouldPlayNotificationSound
         */
        void AddInfo(const char* message, bool shouldPlayNotificationSound);

        /**
         * @brief Show warning message in-game
         * @param message
         * @param shouldPlayNotificationSound
         */
        void AddWarning(const char* message, bool shouldPlayNotificationSound);

        /**
         * @brief Show hint in-game
         * @param message message in brief window
         * @param a2 unknown flag
         * @param shouldPlayNotificationSound - true - play sound
         * @param osdElement - unknown value, but looks like it's not used
         * @param a6 - unknown, but it's used only for M00
         * @param tutorialId - if not null will be used localization from m00/Tutorials/{tutorialId}
         */
        void AddHint(const char* message, bool a2, bool shouldPlayNotificationSound, int osdElement, bool a6, const char* tutorialId);

        //data (total size is 0x8FC, base size is 0x10)
        char m_field10;
        char field_11;
        char field_12;
        char field_13;
        char m_field14;
        char field_15;
        char field_16;
        char field_17;
        char m_field18;
        char field_19;
        char m_bShowWeaponDisplay;
        char field_1B;
        char m_field1C;
        char field_1D;
        char m_bDrawActionMenu;
        char field_1F;
        char m_field20;
        char field_21;
        char field_22;
        char field_23;
        int m_field24;
        int m_field28;
        int m_field2C;
        int m_field30;
        char m_field34;
        char field_35;
        char field_36;
        char field_37;
        Glacier::ZCAMERA *m_camera38;
        int m_field3C;
        int m_field40;
        int m_field44;
        int m_field48;
        int m_field4C_REF;
        int m_field50_REF;
        ZLINEOBJ* m_pSubtitles;
        int m_field58;
        int m_field5C;
        int m_field60;
        int m_field64;
        char m_field68;
        char field_69;
        char m_bForcedBriefingIsOver;
        char field_6B;
        int m_field6C;
        int m_field70;
        int m_field74;
        int m_field78;
        int m_field7C;
        int m_field80;
        int m_field84;
        int m_field88;
        int m_field8C;
        int m_field90;
        int m_field94;
        int m_rWeaponDisplay;
        int m_pWeaponDisplay;
        char m_fieldA0;
        char field_A1;
        char field_A2;
        char field_A3;
        int m_fieldA4;
        Glacier::ZVector3 m_vInfoDisplayPosition;
        int m_rInfoDisplay;
        int m_pInfoDisplay;
        float m_fHitpoints;
        int m_fieldC0_REF;
        int m_pHealthFrame;
        int m_fieldC8_REF;
        int m_fieldCC;
        int m_fieldD0_REF;
        int m_fieldD4_REF;
        int m_fieldD8_REF;
        int m_fieldDC_REF;
        int m_fieldE0_REF;
        int m_fieldE4_REF;
        int m_fieldE8_REF;
        int m_fieldEC;
        int m_fieldF0;
        int m_fieldF4;
        int m_fieldF8;
        int m_fieldFC;
        int m_field100;
        int m_field104;
        int m_field108;
        int m_field10C;
        int m_field110;
        int m_field114;
        int m_field118;
        int m_field11C;
        int m_field120;
        int m_field124;
        int m_field128;
        int m_field12C;
        int m_field130;
        int m_field134;
        int m_field138;
        int m_field13C;
        int m_field140;
        int m_field144;
        int m_field148;
        int m_field14C;
        int m_field150;
        int m_field154;
        int m_field158;
        int m_field15C;
        int m_field160;
        int m_field164;
        int m_field168;
        int m_field16C;
        int m_aActorTensionLevels0;
        int m_aActorTensionLevels1;
        int m_aActorTensionLevels2;
        int m_aActorTensionLevels3;
        int m_aActorTensionLevels4;
        int m_aActorTensionLevels5;
        int m_field188;
        int m_field18C;
        int m_field190;
        int m_field194;
        int m_field198;
        int m_field19C;
        int m_field1A0;
        int m_field1A4;
        int m_rCheatMenu;
	    ZCheatMenu* m_pCheatMenu;
        int m_vPickupMenuPosition;
        int m_field1B4;
        int m_field1B8;
        int m_rPickupMenu;
        int m_pPickupMenu;
        Glacier::ZVector3 m_vDropDisplayPosition;
        int m_rDropDisplay;
        int m_pDropDisplay;
        int m_vActionDisplayPosition;
        int m_field1DC;
        int m_field1E0;
        int m_rActionMenu;
        int m_pActionDisplay;
        int m_field1EC;
        int m_field1F0;
        int m_field1F4;
        int m_rContainerMenu;
        int m_pContainerMenu;
        Glacier::ZSDOwner m_sdOwner;
        int m_field210;
        int m_field214;
        int m_field218;
        int m_field21C;
        int m_field220;
        int m_field224;
        int m_field228;
        int m_field22C;
        int m_field230;
        int m_field234;
        int m_field238;
        int m_field23C;
        int m_field240;
        int m_field244;
        int m_field248;
        int m_field24C;
        int m_field250;
        int m_field254;
        int m_field258;
        int m_field25C;
        int m_field260;
        int m_field264;
        int m_field268;
        int m_field26C;
        int m_field270;
        int m_field274;
        int m_field278;
        int m_field27C;
        int m_field280;
        int m_field284;
        int m_field288;
        int m_field28C;
        int m_field290;
        int m_field294;
        int m_field298;
        int m_field29C;
        int m_field2A0;
        int m_field2A4;
        int m_field2A8;
        int m_field2AC;
        int m_field2B0;
        int m_field2B4;
        int m_field2B8;
        int m_field2BC;
        int m_field2C0;
        int m_field2C4;
        int m_field2C8;
        int m_field2CC;
        int m_field2D0;
        int m_field2D4;
        int m_field2D8;
        int m_field2DC;
        int m_field2E0;
        int m_field2E4;
        int m_field2E8;
        int m_field2EC;
        int m_field2F0;
        int m_field2F4;
        int m_field2F8;
        int m_field2FC;
        int m_field300;
        int m_field304;
        int m_field308;
        int m_field30C;
        int m_field310;
        int m_field314;
        int m_field318;
        int m_field31C;
        int m_field320;
        int m_field324;
        int m_field328;
        int m_field32C;
        int m_field330;
        int m_field334;
        int m_field338;
        int m_field33C;
        int m_field340;
        int m_field344;
        int m_field348;
        int m_field34C;
        int m_field350;
        int m_field354;
        int m_field358;
        int m_field35C;
        int m_field360;
        int m_field364;
        int m_field368;
        int m_field36C;
        int m_field370;
        int m_field374;
        int m_field378;
        int m_field37C;
        int m_field380;
        int m_field384;
        int m_field388;
        int m_field38C;
        int m_field390;
        int m_field394;
        int m_field398;
        int m_field39C;
        int m_field3A0;
        int m_field3A4;
        int m_field3A8;
        int m_field3AC;
        int m_field3B0;
        int m_field3B4;
        int m_field3B8;
        int m_field3BC;
        int m_field3C0;
        int m_field3C4;
        int m_field3C8;
        int m_field3CC;
        int m_field3D0;
        int m_field3D4;
        int m_field3D8;
        int m_field3DC;
        int m_field3E0;
        int m_field3E4;
        int m_field3E8;
        int m_field3EC;
        int m_field3F0;
        int m_field3F4;
        int m_field3F8;
        int m_field3FC;
        int m_field400;
        int m_field404;
        int m_field408;
        int m_field40C;
        int m_field410;
        int m_field414;
        int m_field418;
        int m_field41C;
        int m_field420;
        int m_field424;
        int m_field428;
        int m_field42C;
        int m_field430;
        int m_field434;
        int m_field438;
        int m_field43C;
        int m_field440;
        int m_field444;
        int m_field448;
        int m_field44C;
        int m_field450;
        int m_field454;
        int m_field458;
        int m_field45C;
        int m_field460;
        int m_field464;
        int m_field468;
        int m_field46C;
        int m_field470;
        int m_field474;
        int m_field478;
        int m_field47C;
        int m_field480;
        int m_field484;
        int m_field488;
        int m_field48C;
        int m_field490;
        int m_field494;
        int m_field498;
        int m_field49C;
        int m_field4A0;
        int m_field4A4;
        int m_field4A8;
        int m_field4AC;
        int m_field4B0;
        int m_field4B4;
        int m_field4B8;
        int m_field4BC;
        int m_field4C0;
        int m_field4C4;
        int m_field4C8;
        int m_field4CC;
        int m_field4D0;
        int m_field4D4;
        int m_field4D8;
        int m_field4DC;
        int m_field4E0;
        int m_field4E4;
        int m_field4E8;
        int m_field4EC;
        int m_field4F0;
        int m_field4F4;
        int m_field4F8;
        int m_field4FC;
        int m_field500;
        int m_field504;
        int m_field508;
        int m_field50C;
        int m_field510;
        int m_field514;
        int m_field518;
        int m_field51C;
        int m_field520;
        int m_field524;
        int m_field528;
        int m_field52C;
        int m_field530;
        int m_field534;
        int m_field538;
        int m_field53C;
        int m_field540;
        int m_field544;
        int m_field548;
        int m_field54C;
        int m_field550;
        int m_field554;
        int m_field558;
        int m_field55C;
        int m_field560;
        int m_field564;
        int m_field568;
        int m_field56C;
        int m_field570;
        int m_field574;
        int m_field578;
        int m_field57C;
        int m_field580;
        int m_field584;
        int m_field588;
        int m_field58C;
        int m_field590;
        int m_field594;
        int m_field598;
        int m_field59C;
        int m_field5A0;
        int m_field5A4;
        int m_field5A8;
        int m_field5AC;
        int m_field5B0;
        int m_field5B4;
        int m_field5B8;
        int m_field5BC;
        int m_field5C0;
        int m_field5C4;
        int m_field5C8;
        int m_field5CC;
        int m_field5D0;
        int m_field5D4;
        int m_field5D8;
        int m_field5DC;
        int m_field5E0;
        int m_field5E4;
        int m_field5E8;
        int m_field5EC;
        int m_field5F0;
        int m_field5F4;
        int m_field5F8;
        int m_field5FC;
        int m_field600;
        int m_field604;
        int m_field608;
        int m_field60C;
        int m_field610;
        int m_field614;
        int m_field618;
        int m_field61C;
        int m_field620;
        int m_field624;
        int m_field628;
        int m_field62C;
        int m_field630;
        int m_field634;
        int m_field638;
        int m_field63C;
        int m_field640;
        int m_field644;
        int m_field648;
        int m_field64C;
        int m_field650;
        int m_field654;
        int m_field658;
        int m_field65C;
        int m_field660;
        int m_field664;
        int m_field668;
        int m_field66C;
        int m_field670;
        int m_field674;
        int m_field678;
        int m_field67C;
        int m_field680;
        int m_field684;
        int m_field688;
        int m_field68C;
        int m_field690;
        int m_field694;
        int m_field698;
        int m_field69C;
        int m_field6A0;
        int m_field6A4;
        int m_field6A8;
        int m_field6AC;
        int m_field6B0;
        int m_field6B4;
        int m_field6B8;
        int m_field6BC;
        int m_field6C0;
        int m_field6C4;
        int m_field6C8;
        int m_field6CC;
        int m_field6D0;
        int m_field6D4;
        int m_field6D8;
        int m_field6DC;
        int m_field6E0;
        int m_field6E4;
        int m_field6E8;
        int m_field6EC;
        int m_field6F0;
        int m_field6F4;
        int m_field6F8;
        int m_field6FC;
        int m_field700;
        int m_field704;
        int m_field708;
        int m_field70C;
        int m_field710;
        int m_field714;
        int m_field718;
        int m_field71C;
        int m_field720;
        int m_field724;
        int m_field728;
        int m_field72C;
        int m_field730;
        int m_field734;
        int m_field738;
        int m_field73C;
        int m_field740;
        int m_field744;
        int m_field748;
        int m_field74C;
        int m_field750;
        int m_field754;
        int m_field758;
        int m_field75C;
        int m_field760;
        int m_field764;
        int m_field768;
        int m_field76C;
        int m_field770;
        int m_field774;
        int m_field778;
        int m_field77C;
        int m_field780;
        int m_field784;
        int m_field788;
        int m_field78C;
        int m_field790;
        int m_field794;
        int m_field798;
        int m_field79C;
        int m_field7A0;
        int m_field7A4;
        int m_field7A8;
        int m_field7AC;
        int m_field7B0;
        int m_field7B4;
        int m_field7B8;
        int m_field7BC;
        int m_field7C0;
        int m_field7C4;
        int m_field7C8;
        int m_field7CC;
        int m_field7D0;
        int m_field7D4;
        int m_field7D8;
        int m_field7DC;
        int m_field7E0;
        int m_field7E4;
        int m_field7E8;
        int m_field7EC;
        int m_field7F0;
        int m_field7F4;
        int m_field7F8;
        int m_field7FC;
        int m_field800;
        int m_field804;
        int m_field808;
        int m_field80C;
        int m_field810;
        int m_field814;
        int m_field818;
        int m_field81C;
        int m_field820;
        int m_field824;
        int m_field828;
        int m_field82C;
        int m_field830;
        int m_field834;
        int m_field838;
        int m_field83C;
        int m_field840;
        int m_field844;
        int m_field848;
        int m_field84C;
        int m_field850;
        int m_field854;
        int m_field858;
        int m_field85C;
        int m_field860;
        int m_field864;
        int m_field868;
        int m_field86C;
        int m_field870;
        int m_field874;
        int m_field878;
        int m_field87C;
        int m_field880;
        int m_field884;
        int m_field888;
        int m_field88C;
        int m_field890;
        int m_field894;
        int m_field898;
        int m_field89C;
        int m_field8A0;
        int m_field8A4;
        int m_field8A8;
        int m_field8AC;
        int m_field8B0;
        int m_field8B4;
        int m_field8B8;
        int m_field8BC;
        int m_field8C0;
        int m_field8C4;
        int m_field8C8;
        int m_field8CC;
        int m_field8D0;
        int m_field8D4;
        int m_field8D8;
        int m_field8DC;
        int m_field8E0;
        int m_field8E4;
        int m_field8E8;
        int m_field8EC;
        int m_field8F0;
        int m_field8F4;
        char m_field8F8;
        char field_8F9;
        char field_8FA;
        char field_8FB;
    };
}