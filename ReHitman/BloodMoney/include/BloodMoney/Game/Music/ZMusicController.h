#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/REFTAB32.h>

namespace Hitman::BloodMoney
{
    struct SPlayListEntry
    {
        int m_field0;
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
        int m_field18;
        int m_field1C;
        int m_field20;
        int m_field24;
        int m_field28;
        int m_field2C;
        unsigned short m_field30;
        unsigned short m_field32;
        int m_field34;
        bool m_field38;
        bool m_field39;
        bool m_field3A;
        bool m_field3B;
    }; //Size is 0x3C

    class ZMusicController
    {
    public:
        static constexpr size_t kMaxPlayListEntries = 0xC;

        //vftable
        virtual void SetLocation(Glacier::ZREF ref, bool);

        //TODO: Reverse member methods

        //data
        Glacier::REFTAB32 m_reftab32_4;
        Glacier::REFTAB32 m_reftab32_B0;
        int m_field15C;
        int m_field160;
        int m_field164;
        int m_field168;
        char m_field16C;
        char field_16D;
        char field_16E;
        char field_16F;
        int m_eControllerState;
        int m_field174;
        int m_field178;
        int m_field17C;
        int m_field180;
        int m_field184;
        char m_field188;
        char field_189;
        char field_18A;
        char field_18B;
        int m_field18C;
        float m_field190;
        int m_field194;
        int m_field198;
        int m_field19C;
        int m_field1A0;
        char m_field1A4;
        char field_1A5;
        char field_1A6;
        char field_1A7;
        char m_field1A8;
        char field_1A9;
        char field_1AA;
        char field_1AB;
        char m_field1AC;
        char field_1AD;
        char field_1AE;
        char field_1AF;
        int m_field1B0;
        int m_field1B4;
        int m_field1B8;
        int m_field1BC;
        int m_field1C0;
        int m_field1C4;
        int m_field1C8;
        int m_field1CC;
        char m_field1D0;
        char field_1D1;
        char field_1D2;
        char field_1D3;
        int m_field1D4;
        int m_field1D8;
        char m_field1DC;
        char field_1DD;
        char field_1DE;
        char field_1DF;
        int m_field1E0;
        int m_field1E4;
        int m_field1E8;
        int m_field1EC;
        short m_field1F0;
        short field_1F2;
        int m_field1F4;
        int m_field1F8;
        int m_field1FC;
        int m_field200;
        int m_field204;
        float m_field208;
        int m_field20C;
        int m_field210;
        char m_field214;
        char field_215;
        char field_216;
        char field_217;
        int m_field218;
        SPlayListEntry m_playlist_21C[kMaxPlayListEntries];
        SPlayListEntry m_playlist_4EC[kMaxPlayListEntries];
        SPlayListEntry m_playlist_7BC[kMaxPlayListEntries];
        int m_fieldA8C;
        int m_fieldA90;
        int m_fieldA94;
        int m_fieldA98;
        int m_fieldA9C;
        int m_fieldAA0;
        int m_fieldAA4;
        int m_fieldAA8;
        int m_fieldAAC;
        int m_fieldAB0;
        int m_fieldAB4;
        int m_fieldAB8;
        int m_fieldABC;
        int m_fieldAC0;
        int m_fieldAC4;
        int m_fieldAC8;
        int m_fieldACC;
        int m_fieldAD0;
        int m_fieldAD4;
        int m_fieldAD8;
        int m_fieldADC;
        int m_fieldAE0;
        int m_fieldAE4;
        int m_fieldAE8;
        int m_fieldAEC;
        int m_fieldAF0;
        int m_fieldAF4;
        int m_fieldAF8;
        int m_fieldAFC;
        int m_fieldB00;
        int m_fieldB04;
        int m_fieldB08;
        int m_fieldB0C;
        int m_fieldB10;
        int m_fieldB14;
        int m_fieldB18;
        int m_fieldB1C;
        int m_fieldB20;
        int m_fieldB24;
        int m_fieldB28;
        int m_fieldB2C;
        int m_fieldB30;
        int m_fieldB34;
        int m_fieldB38;
        int m_fieldB3C;
        int m_fieldB40;
        int m_fieldB44;
        int m_fieldB48;
        int m_fieldB4C;
        int m_fieldB50;
        int m_fieldB54;
        int m_fieldB58;
        int m_fieldB5C;
        int m_fieldB60;
        int m_fieldB64;
        int m_fieldB68;
        int m_fieldB6C;
        int m_fieldB70;
        int m_fieldB74;
        int m_fieldB78;
        int m_fieldB7C;
        int m_fieldB80;
        int m_fieldB84;
        int m_fieldB88;
        int m_fieldB8C;
        int m_fieldB90;
        int m_fieldB94;
        int m_fieldB98;
        int m_fieldB9C;
        int m_fieldBA0;
        int m_fieldBA4;
        int m_fieldBA8;
        int m_fieldBAC;
        int m_fieldBB0;
        int m_fieldBB4;
        int m_fieldBB8;
        int m_fieldBBC;
        int m_fieldBC0;
        int m_fieldBC4;
        int m_fieldBC8;
        int m_fieldBCC;
        int m_fieldBD0;
        int m_fieldBD4;
        int m_fieldBD8;
        int m_fieldBDC;
        int m_fieldBE0;
        int m_fieldBE4;
        int m_fieldBE8;
        int m_fieldBEC;
        int m_fieldBF0;
        int m_fieldBF4;
        int m_fieldBF8;
        int m_fieldBFC;
        int m_fieldC00;
        int m_fieldC04;
        int m_fieldC08;
        int m_fieldC0C;
        int m_fieldC10;
        int m_fieldC14;
        int m_fieldC18;
        int m_fieldC1C;
        int m_fieldC20;
        int m_fieldC24;
        int m_fieldC28;
        int m_fieldC2C;
        int m_fieldC30;
        int m_fieldC34;
        int m_fieldC38;
        int m_fieldC3C;
        int m_fieldC40;
        int m_fieldC44;
        int m_fieldC48;
        int m_fieldC4C;
        int m_fieldC50;
        int m_fieldC54;
        int m_fieldC58;
        int m_fieldC5C;
        int m_fieldC60;
        int m_fieldC64;
        int m_fieldC68;
        int m_fieldC6C;
        int m_fieldC70;
        int m_fieldC74;
        int m_fieldC78;
        int m_fieldC7C;
        int m_fieldC80;
        int m_fieldC84;
        int m_fieldC88;
        int m_fieldC8C;
        int m_fieldC90;
        int m_fieldC94;
        int m_fieldC98;
        int m_fieldC9C;
        int m_fieldCA0;
        int m_fieldCA4;
        int m_fieldCA8;
        int m_fieldCAC;
        int m_fieldCB0;
        int m_fieldCB4;
        int m_fieldCB8;
        int m_fieldCBC;
        int m_fieldCC0;
        int m_fieldCC4;
        int m_fieldCC8;
        int m_fieldCCC;
        int m_fieldCD0;
        int m_fieldCD4;
        int m_fieldCD8;
        int m_fieldCDC;
        int m_fieldCE0;
        int m_fieldCE4;
        int m_fieldCE8;
        int m_fieldCEC;
        int m_fieldCF0;
        int m_fieldCF4;
        int m_fieldCF8;
        int m_fieldCFC;
        int m_fieldD00;
        int m_fieldD04;
        int m_fieldD08;
        int m_fieldD0C;
        int m_fieldD10;
        int m_fieldD14;
        int m_fieldD18;
        int m_fieldD1C;
        int m_fieldD20;
        int m_fieldD24;
        int m_fieldD28;
        int m_fieldD2C;
        int m_fieldD30;
        int m_fieldD34;
        int m_fieldD38;
        int m_fieldD3C;
        int m_fieldD40;
        int m_fieldD44;
        int m_fieldD48;
        int m_fieldD4C;
        int m_fieldD50;
        int m_fieldD54;
        int m_fieldD58;
        int m_fieldD5C;
        int m_fieldD60;
        int m_fieldD64;
        int m_fieldD68;
        int m_fieldD6C;
        int m_fieldD70;
        int m_fieldD74;
        int m_fieldD78;
        int m_fieldD7C;
        int m_fieldD80;
        int m_fieldD84;
        int m_fieldD88;
        int m_fieldD8C;
        int m_fieldD90;
        int m_fieldD94;
        int m_fieldD98;
        int m_fieldD9C;
        int m_fieldDA0;
        int m_fieldDA4;
        int m_fieldDA8;
        int m_fieldDAC;
        int m_fieldDB0;
        int m_fieldDB4;
        int m_fieldDB8;
        int m_fieldDBC;
        int m_fieldDC0;
        int m_fieldDC4;
        int m_fieldDC8;
        int m_fieldDCC;
        int m_fieldDD0;
        int m_fieldDD4;
        int m_fieldDD8;
        int m_fieldDDC;
        int m_fieldDE0;
        int m_fieldDE4;
        int m_fieldDE8;
        int m_fieldDEC;
        int m_fieldDF0;
        int m_fieldDF4;
        int m_fieldDF8;
        int m_fieldDFC;
        int m_fieldE00;
        int m_fieldE04;
        int m_fieldE08;
        int m_fieldE0C;
        int m_fieldE10;
        int m_fieldE14;
        int m_fieldE18;
        int m_fieldE1C;
        int m_fieldE20;
        int m_fieldE24;
        int m_fieldE28;
        int m_fieldE2C;
        int m_fieldE30;
        int m_fieldE34;
        int m_fieldE38;
        int m_fieldE3C;
        int m_fieldE40;
        int m_fieldE44;
        int m_fieldE48;
        int m_fieldE4C;
        int m_fieldE50;
        int m_fieldE54;
        int m_fieldE58;
        int m_fieldE5C;
        int m_fieldE60;
        int m_fieldE64;
        int m_fieldE68;
        int m_fieldE6C;
        int m_fieldE70;
        int m_fieldE74;
        int m_fieldE78;
        int m_fieldE7C;
        int m_fieldE80;
        int m_fieldE84;
        int m_fieldE88;
        int m_fieldE8C;
        int m_fieldE90;
        int m_fieldE94;
        int m_fieldE98;
        int m_fieldE9C;
        int m_fieldEA0;
        int m_fieldEA4;
        int m_fieldEA8;
        int m_fieldEAC;
        int m_fieldEB0;
        int m_fieldEB4;
        int m_fieldEB8;
        int m_fieldEBC;
        int m_fieldEC0;
        int m_fieldEC4;
        int m_fieldEC8;
        int m_fieldECC;
        int m_fieldED0;
        int m_fieldED4;
        int m_fieldED8;
        int m_fieldEDC;
        int m_fieldEE0;
        int m_fieldEE4;
        int m_fieldEE8;
        int m_fieldEEC;
        int m_fieldEF0;
        int m_fieldEF4;
        int m_fieldEF8;
        int m_fieldEFC;
        int m_fieldF00;
        int m_fieldF04;
        int m_fieldF08;
        int m_fieldF0C;
        int m_fieldF10;
        int m_fieldF14;
        int m_fieldF18;
        int m_fieldF1C;
        int m_fieldF20;
        int m_fieldF24;
        int m_fieldF28;
        int m_fieldF2C;
        int m_fieldF30;
        int m_fieldF34;
        int m_fieldF38;
        int m_fieldF3C;
        int m_fieldF40;
        int m_fieldF44;
        int m_fieldF48;
        int m_fieldF4C;
        int m_fieldF50;
        int m_fieldF54;
        int m_fieldF58;
        int m_fieldF5C;
        int m_fieldF60;
        int m_fieldF64;
        int m_fieldF68;
        int m_fieldF6C;
        int m_fieldF70;
        int m_fieldF74;
        int m_fieldF78;
        int m_fieldF7C;
        int m_fieldF80;
        int m_fieldF84;
        int m_fieldF88;
        int m_fieldF8C;
        int m_fieldF90;
        int m_fieldF94;
        int m_fieldF98;
        int m_fieldF9C;
        int m_fieldFA0;
        int m_fieldFA4;
        int m_fieldFA8;
        int m_fieldFAC;
        int m_fieldFB0;
        int m_fieldFB4;
        int m_fieldFB8;
        int m_fieldFBC;
        int m_fieldFC0;
        int m_fieldFC4;
        int m_fieldFC8;
        int m_fieldFCC;
        int m_fieldFD0;
        int m_fieldFD4;
        int m_fieldFD8;
        int m_fieldFDC;
        int m_fieldFE0;
        int m_fieldFE4;
        int m_fieldFE8;
        int m_fieldFEC;
        int m_fieldFF0;
        int m_fieldFF4;
        int m_fieldFF8;
        int m_fieldFFC;
        int m_field1000;
        int m_field1004;
        int m_field1008;
        int m_field100C;
        int m_field1010;
        int m_field1014;
        int m_field1018;
        int m_field101C;
        int m_field1020;
        int m_field1024;
        int m_field1028;
        int m_field102C;
        int m_field1030;
        int m_field1034;
        int m_field1038;
        int m_field103C;
        int m_field1040;
        int m_field1044;
        int m_field1048;
        int m_field104C;
        int m_field1050;
        int m_field1054;
        int m_field1058;
        int m_field105C;
        int m_field1060;
        int m_field1064;
        int m_field1068;
        int m_field106C;
        int m_field1070;
        int m_field1074;
        int m_field1078;
        int m_field107C;
        int m_field1080;
        int m_field1084;
        int m_field1088;
        int m_field108C;
        int m_field1090;
        int m_field1094;
        int m_field1098;
        int m_field109C;
        int m_field10A0;
        int m_field10A4;
        int m_field10A8;
        int m_field10AC;
        int m_field10B0;
        int m_field10B4;
        int m_field10B8;
        int m_field10BC;
        int m_field10C0;
        int m_field10C4;
        int m_field10C8;
        int m_field10CC;
        int m_field10D0;
        int m_field10D4;
        int m_field10D8;
        int m_field10DC;
        int m_field10E0;
        int m_field10E4;
        int m_field10E8;
        int m_field10EC;
        int m_field10F0;
        int m_field10F4;
        int m_field10F8;
        int m_field10FC;
        int m_field1100;
        int m_field1104;
        int m_field1108;
        int m_field110C;
        int m_field1110;
        int m_field1114;
        int m_field1118;
        int m_field111C;
        int m_field1120;
        int m_field1124;
        int m_field1128;
        int m_field112C;
        int m_field1130;
        int m_field1134;
        int m_field1138;
        int m_field113C;
        int m_field1140;
        int m_field1144;
        int m_field1148;
        int m_field114C;
        int m_field1150;
        int m_field1154;
        int m_field1158;
        int m_field115C;
        int m_field1160;
        int m_field1164;
        int m_field1168;
        int m_field116C;
        int m_field1170;
        int m_field1174;
        int m_field1178;
        int m_field117C;
        int m_field1180;
        int m_field1184;
        int m_field1188;
        int m_field118C;
        int m_field1190;
        int m_field1194;
        int m_field1198;
        int m_field119C;
        int m_field11A0;
        int m_field11A4;
        int m_field11A8;
        int m_field11AC;
        int m_field11B0;
        int m_field11B4;
        int m_field11B8;
        int m_field11BC;
        int m_field11C0;
        int m_field11C4;
        int m_field11C8;
        int m_field11CC;
        int m_field11D0;
        int m_field11D4;
        int m_field11D8;
        int m_field11DC;
        int m_field11E0;
        int m_field11E4;
        int m_field11E8;
        int m_field11EC;
        int m_field11F0;
        int m_field11F4;
        int m_field11F8;
        int m_field11FC;
        int m_field1200;
        int m_field1204;
        int m_field1208;
        int m_field120C;
        int m_field1210;
        int m_field1214;
        int m_field1218;
        int m_field121C;
        int m_field1220;
        int m_field1224;
        int m_field1228;
        int m_field122C;
        int m_field1230;
        int m_field1234;
        int m_field1238;
        int m_field123C;
        int m_field1240;
        int m_field1244;
        int m_field1248;
        int m_field124C;
        int m_field1250;
        int m_field1254;
        int m_field1258;
        int m_field125C;
        int m_field1260;
        int m_field1264;
        int m_field1268;
        int m_field126C;
        int m_field1270;
        int m_field1274;
        int m_field1278;
        int m_field127C;
        int m_field1280;
        int m_field1284;
        int m_field1288;
        int m_field128C;
        int m_field1290;
        int m_field1294;
        int m_field1298;
        int m_field129C;
        int m_field12A0;
        int m_field12A4;
        int m_field12A8;
        int m_field12AC;
        int m_field12B0;
        int m_field12B4;
        int m_field12B8;
        int m_field12BC;
        int m_field12C0;
        int m_field12C4;
        int m_field12C8;
        int m_field12CC;
        int m_field12D0;
        int m_field12D4;
        int m_field12D8;
        int m_field12DC;
        int m_field12E0;
        int m_field12E4;
        int m_field12E8;
        int m_field12EC;
        int m_field12F0;
        int m_field12F4;
        int m_field12F8;
        int m_field12FC;
        int m_field1300;
        int m_field1304;
        int m_field1308;
        int m_field130C;
        int m_field1310;
        int m_field1314;
        int m_field1318;
        int m_field131C;
        int m_field1320;
        int m_field1324;
        int m_field1328;
        int m_field132C;
        int m_field1330;
        int m_field1334;
        int m_field1338;
        int m_field133C;
        int m_field1340;
        int m_field1344;
        int m_field1348;
        int m_field134C;
        int m_field1350;
        int m_field1354;
        int m_field1358;
        int m_field135C;
        int m_field1360;
        int m_field1364;
        int m_field1368;
        int m_field136C;
        int m_field1370;
        int m_field1374;
        int m_field1378;
        int m_field137C;
        int m_field1380;
        int m_field1384;
        int m_field1388;
        int m_field138C;
        int m_field1390;
        int m_field1394;
        int m_field1398;
        int m_field139C;
        int m_field13A0;
        int m_field13A4;
        int m_field13A8;
        int m_field13AC;
        int m_field13B0;
        int m_field13B4;
        int m_field13B8;
        int m_field13BC;
        int m_field13C0;
        int m_field13C4;
        int m_field13C8;
        int m_field13CC;
        int m_field13D0;
        int m_field13D4;
        int m_field13D8;
        int m_field13DC;
        int m_field13E0;
        int m_field13E4;
        int m_field13E8;
        int m_field13EC;
        int m_field13F0;
        int m_field13F4;
        int m_field13F8;
        int m_field13FC;
        int m_field1400;
        int m_field1404;
        int m_field1408;
        int m_field140C;
        int m_field1410;
        int m_field1414;
        int m_field1418;
        int m_field141C;
        int m_field1420;
        int m_field1424;
        int m_field1428;
        int m_field142C;
        int m_field1430;
        int m_field1434;
        int m_field1438;
        int m_field143C;
        int m_field1440;
        int m_field1444;
        int m_field1448;
        int m_field144C;
        int m_field1450;
        int m_field1454;
        int m_field1458;
        int m_field145C;
        int m_field1460;
        int m_field1464;
        int m_field1468;
        int m_field146C;
        int m_field1470;
        int m_field1474;
        int m_field1478;
        int m_field147C;
        int m_field1480;
        int m_field1484;
        int m_field1488;
        int m_field148C;
        int m_field1490;
        int m_field1494;
        int m_field1498;
        int m_field149C;
        int m_field14A0;
        int m_field14A4;
        int m_field14A8;
        int m_field14AC;
        int m_field14B0;
        int m_field14B4;
        int m_field14B8;
        int m_field14BC;
        int m_field14C0;
        int m_field14C4;
        int m_field14C8;
        int m_field14CC;
        int m_field14D0;
        int m_field14D4;
        int m_field14D8;
        int m_field14DC;
        int m_field14E0;
        int m_field14E4;
        int m_field14E8;
        int m_field14EC;
        int m_field14F0;
        int m_field14F4;
        int m_field14F8;
        int m_field14FC;
        int m_field1500;
        int m_field1504;
        int m_field1508;
        int m_field150C;
        int m_field1510;
        int m_field1514;
        int m_field1518;
        int m_field151C;
        int m_field1520;
        int m_field1524;
        int m_field1528;
        int m_field152C;
        int m_field1530;
        int m_field1534;
        int m_field1538;
        int m_field153C;
        int m_field1540;
        int m_field1544;
        int m_field1548;
        int m_field154C;
        int m_field1550;
        int m_field1554;
        int m_field1558;
        int m_field155C;
        int m_field1560;
        int m_field1564;
        int m_field1568;
        int m_field156C;
        int m_field1570;
        int m_field1574;
        int m_field1578;
        int m_field157C;
        int m_field1580;
        int m_field1584;
        int m_field1588;
        int m_field158C;
        int m_field1590;
        int m_field1594;
        int m_field1598;
        int m_field159C;
        int m_field15A0;
        int m_field15A4;
        int m_field15A8;
        int m_field15AC;
        int m_field15B0;
        int m_field15B4;
        int m_field15B8;
        int m_field15BC;
        int m_field15C0;
        int m_field15C4;
        int m_field15C8;
        int m_field15CC;
        int m_field15D0;
        int m_field15D4;
        int m_field15D8;
        int m_field15DC;
        int m_field15E0;
        int m_field15E4;
        int m_field15E8;
        int m_field15EC;
        int m_field15F0;
        int m_field15F4;
        int m_field15F8;
        int m_field15FC;
        int m_field1600;
        int m_field1604;
        int m_field1608;
        int m_field160C;
        int m_field1610;
        int m_field1614;
        int m_field1618;
        int m_field161C;
        int m_field1620;
        int m_field1624;
        int m_field1628;
        int m_field162C;
        int m_field1630;
        int m_field1634;
        int m_field1638;
        int m_field163C;
        int m_field1640;
        int m_field1644;
        int m_field1648;
        int m_field164C;
        int m_field1650;
        int m_field1654;
        int m_field1658;
        int m_field165C;
        int m_field1660;
        int m_field1664;
        int m_field1668;
        int m_field166C;
        int m_field1670;
        int m_field1674;
        int m_field1678;
        int m_field167C;
        int m_field1680;
        int m_field1684;
        int m_field1688;
        int m_field168C;
        int m_field1690;
        int m_field1694;
        int m_field1698;
        int m_field169C;
        int m_field16A0;
        int m_field16A4;
        int m_field16A8;
        int m_field16AC;
        int m_field16B0;
        int m_field16B4;
        int m_field16B8;
        int m_field16BC;
        int m_field16C0;
        int m_field16C4;
        int m_field16C8;
        int m_field16CC;
        int m_field16D0;
        int m_field16D4;
        int m_field16D8;
        int m_field16DC;
        int m_field16E0;
        int m_field16E4;
        int m_field16E8;
        int m_field16EC;
        int m_field16F0;
        int m_field16F4;
        int m_field16F8;
        int m_field16FC;
        int m_field1700;
        int m_field1704;
        int m_field1708;
        int m_field170C;
        int m_field1710;
        int m_field1714;
        int m_field1718;
        int m_field171C;
        int m_field1720;
        int m_field1724;
        int m_field1728;
        int m_field172C;
        int m_field1730;
        int m_field1734;
        int m_field1738;
        int m_field173C;
        int m_field1740;
        int m_field1744;
        int m_field1748;
        int m_field174C;
        int m_field1750;
        int m_field1754;
        int m_field1758;
        int m_field175C;
        int m_field1760;
        int m_field1764;
        int m_field1768;
        int m_field176C;
        int m_field1770;
        int m_field1774;
        int m_field1778;
        int m_field177C;
        int m_field1780;
        int m_field1784;
        int m_field1788;
        int m_field178C;
        int m_field1790;
        int m_field1794;
        int m_field1798;
        int m_field179C;
        int m_field17A0;
        int m_field17A4;
        int m_field17A8;
        int m_field17AC;
        int m_field17B0;
        int m_field17B4;
        int m_field17B8;
        int m_field17BC;
        int m_field17C0;
        int m_field17C4;
        int m_field17C8;
        int m_field17CC;
        int m_field17D0;
        int m_field17D4;
        int m_field17D8;
        int m_field17DC;
        int m_field17E0;
        int m_field17E4;
        int m_field17E8;
        int m_field17EC;
        int m_field17F0;
        int m_field17F4;
        int m_field17F8;
        int m_field17FC;
        int m_field1800;
        int m_field1804;
        int m_field1808;
        int m_field180C;
        int m_field1810;
        int m_field1814;
        int m_field1818;
        int m_field181C;
        int m_field1820;
        int m_field1824;
        int m_field1828;
        int m_field182C;
        int m_field1830;
        int m_field1834;
        int m_field1838;
        int m_field183C;
        int m_field1840;
        int m_field1844;
        int m_field1848;
        int m_field184C;
        int m_field1850;
        int m_field1854;
        int m_field1858;
        int m_field185C;
        int m_field1860;
        int m_field1864;
        int m_field1868;
        int m_field186C;
        int m_field1870;
        int m_field1874;
        int m_field1878;
        int m_field187C;
        int m_field1880;
        int m_field1884;
        int m_field1888;
        int m_field188C;
        int m_field1890;
        int m_field1894;
        int m_field1898;
        int m_field189C;
        int m_field18A0;
        int m_field18A4;
        int m_field18A8;
        int m_field18AC;
        int m_field18B0;
        int m_field18B4;
        int m_field18B8;
        int m_field18BC;
        int m_field18C0;
        int m_field18C4;
        int m_field18C8;
        int m_field18CC;
        int m_field18D0;
        int m_field18D4;
        int m_field18D8;
        int m_field18DC;
        int m_field18E0;
        int m_field18E4;
        int m_field18E8;
        int m_field18EC;
        int m_field18F0;
        int m_field18F4;
        int m_field18F8;
        int m_field18FC;
        int m_field1900;
        int m_field1904;
        int m_field1908;
        int m_field190C;
        int m_field1910;
        int m_field1914;
        int m_field1918;
        int m_field191C;
        int m_field1920;
        int m_field1924;
        int m_field1928;
        int m_field192C;
        int m_field1930;
        int m_field1934;
        int m_field1938;
        int m_field193C;
        int m_field1940;
        int m_field1944;
        int m_field1948;
        int m_field194C;
        int m_field1950;
        int m_field1954;
        int m_field1958;
        int m_field195C;
        int m_field1960;
        int m_field1964;
        int m_field1968;
        int m_field196C;
        int m_field1970;
        int m_field1974;
        int m_field1978;
        int m_field197C;
        int m_field1980;
        int m_field1984;
        int m_field1988;
        int m_field198C;
        int m_field1990;
        int m_field1994;
        int m_field1998;
        int m_field199C;
        int m_field19A0;
        int m_field19A4;
        int m_field19A8;
        int m_field19AC;
        int m_field19B0;
        int m_field19B4;
        int m_field19B8;
        int m_field19BC;
        int m_field19C0;
        int m_field19C4;
        int m_field19C8;
        int m_field19CC;
        int m_field19D0;
        int m_field19D4;
        int m_field19D8;
        int m_field19DC;
        int m_field19E0;
        int m_field19E4;
        int m_field19E8;
        int m_field19EC;
        int m_field19F0;
        int m_field19F4;
        int m_field19F8;
        int m_field19FC;
        int m_field1A00;
        int m_field1A04;
        int m_field1A08;
        int m_field1A0C;
        int m_field1A10;
        int m_field1A14;
        int m_field1A18;
        int m_field1A1C;
        int m_field1A20;
        int m_field1A24;
        int m_field1A28;
        int m_field1A2C;
        int m_field1A30;
        int m_field1A34;
        int m_field1A38;
        int m_field1A3C;
        int m_field1A40;
        int m_field1A44;
        int m_field1A48;
        SPlayListEntry m_playlistEntry_1A4C;
        SPlayListEntry m_playlistEntry_1A88;
        int m_field1AC4;
        int m_field1AC8;
        int m_field1ACC;
        int m_field1AD0;
        int m_field1AD4;
        int m_field1AD8;
        int m_field1ADC;
        int m_field1AE0;
        int m_field1AE4;
        int m_field1AE8;
        int m_field1AEC;
        int m_field1AF0;
        short m_field1AF4;
        char field_1AF6;
        char field_1AF7;
        int m_field1AF8;
        char m_field1AFC;
        char field_1AFD;
        char field_1AFE;
        char field_1AFF;
        int m_field1B00;
        char m_field1B04;
        char field_1B05;
        char field_1B06;
        char field_1B07;
        char m_buffer0xFC_at_1B08[252];
        int m_field1C04;
        char m_field1C08;
        char field_1C09;
        char field_1C0A;
        char field_1C0B;
        int m_field1C0C;
        int m_field1C10;
        int m_field1C14;
        int m_field1C18;
        int m_field1C1C;
        int m_field1C20;
        char m_field1C24;
        char field_1C25;
        char field_1C26;
        char field_1C27;
        int m_field1C28;
        int m_field1C2C;
        int m_field1C30;
        int m_field1C34;
        int m_field1C38;
        int m_field1C3C;
        int m_field1C40;
        char m_field1C44;
        char field_1C45;
        char field_1C46;
        char field_1C47;
        int m_field1C48;
        int m_field1C4C;
        bool m_bDisableMusic;
        char field_1C51;
        char field_1C52;
        char field_1C53;
        int m_field1C54;
        int m_field1C58;
        int m_field1C5C;
        int m_field1C60;
        int m_field1C64;
        int m_field1C68;
        int m_field1C6C;
        int m_field1C70;
        int m_field1C74;
        int m_field1C78;
        int m_field1C7C;
        int m_field1C80;
        char m_field1C84;
        char field_1C85;
        char field_1C86;
        char field_1C87;
    }; //size is 0x1C88
}