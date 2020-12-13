#pragma once

#include <Glacier/ZCAMERA.h>

namespace Hitman::BloodMoney
{
    class ZSaveGame;
    class ZLoadGame;

    class ZHM3OverlayCameraClass : public Glacier::ZCAMERA
    {
    public:
        //vftable
        virtual void Save(ZSaveGame*); //nullstub
        virtual void Load(ZLoadGame*); //nullstub

        //data (total size is 0x228, ZCAMERA size is 0x18C)
        Glacier::ZMSGID m_MSG_GetMatPosFrameNr;
        Glacier::ZMSGID m_MSG_Activate;
        int m_field190;
        int m_field194;
        int m_field198;
        int m_field19C;
        int m_field1A0;
        int m_field1A4;
        int m_field1A8;
        int m_field1AC;
        int m_field1B0;
        int m_field1B4;
        int m_field1B8;
        int m_field1BC;
        int m_field1C0;
        int m_field1C4;
        int m_field1C8;
        int m_field1CC;
        int m_field1D0;
        int m_field1D4;
        int m_field1D8;
        int m_field1DC;
        int m_field1E0;
        int m_field1E4;
        int m_field1E8;
        int m_field1EC;
        int m_field1F0;
        int m_field1F4;
        int m_field1F8;
        int m_field1FC;
        int m_field200;
        int m_field204;
        int m_field208;
        int m_field20C;
        int m_field210;
        int m_field214;
        int m_field218;
        int m_field21C;
        int m_field220;
        int m_field224;
    };
}