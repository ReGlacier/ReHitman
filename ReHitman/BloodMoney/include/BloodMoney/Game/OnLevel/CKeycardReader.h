#pragma once

#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
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

    enum eControlledObjecttype
    {
        eDoor = 0,
        eElevator = 1,
    };

    class CKeycardReader : public Glacier::CBaseEvent<Glacier::ZGEOM>
    {
    public:
        //vftable (no changes)
        // custom API
        Glacier::ZBaseGeom* GetNearestDoor(unsigned int iDoorsCount, Glacier::ZBaseGeom** ppDoorsList);
        void ValidateKeyCard(Glacier::ZREF cardREF);
        void InvalidateKeyCard(Glacier::ZREF cardREF);
        void DetermineLnkObjLocation(sQueryLnkObjLocation* pQuery);
        Glacier::ZREF GetKeyCard(Glacier::ZREF playerREF, bool* hasCard);
        Glacier::ZREF GetItemTemplateFromName(const char* itemName);
        bool IsKeyCardValid(BloodMoney::ZHM3Item* pItem);

        // data (total size is 0x8C, base size is 0x30)
        Glacier::ZAction* m_pAction;
        Glacier::ZREF m_rControlledObject;
        Glacier::ZRTString m_szKeyCardTemplates;
        Glacier::ZRTString m_szKeyCard;
        Glacier::ZMSGID m_msgObjectActivator;
        Glacier::ZAUDIOREF m_sValidCardSound;
        Glacier::ZAUDIOREF m_sInvalidCardSound;
        Glacier::REFTAB m_rValidKeyCardTemplate;
        Glacier::REFTAB m_rValidKeycardItems;
        bool m_bElevRequested;
        bool m_bUseLnkAction;
        eControlledObjecttype m_eObjectType;
    };
    RE_VERIFY_SIZE(CKeycardReader, 0x8C); // Verified
}