#pragma once

#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/GlacierFWD.h>

#include <BloodMoney/Game/Items/ZHM3Item.h>

namespace Hitman::BloodMoney
{
    /**
     * @msg MSG_DETERMINELNKOBJLOC
     */
    struct sQueryLnkObjLocation {
        Glacier::ZREF m_geomToDetermineREF; //in
        bool m_bLocatedNearToKeycardReader; //out
    };

    class CKeycardReader : public Glacier::ZEventBase
    {
    public:
        //vftable (no changes)
        // custom API
        Glacier::ZEntityLocator* GetNearestDoor(unsigned int iDoorsCount, Glacier::ZEntityLocator** ppDoorsList);
        void ValidateKeyCard(Glacier::ZREF cardREF);
        void InvalidateKeyCard(Glacier::ZREF cardREF);
        void DetermineLnkObjLocation(sQueryLnkObjLocation* pQuery);
        Glacier::ZREF GetKeyCard(Glacier::ZREF playerREF, bool* hasCard);
        Glacier::ZREF GetItemTemplateFromName(const char* itemName);
        bool IsKeyCardValid(BloodMoney::ZHM3Item* pItem);

        // data (total size is 0x8C, base size is 0x30)
        Glacier::ZREF m_useKeycardActionREF; // REF to Event (use ZEventBuffer to locate instance)
        int m_nearestDoor;
        int m_field38;
        int m_field3C;
        unsigned short m_field40;
        unsigned short m_field42;
        int m_field44;
        int m_field48;
        Glacier::REFTAB m_reftab4C;
        Glacier::REFTAB m_reftab68;
        char m_field84;
        char m_field85;
        char m_field86;
        char m_field87;
        int m_bIsElevatorButton;
    };
}