#include <SI/SI_Silevelcontrol.h>
#include <Glacier/Physics/Fysix/ZWaterBoxManager.h>
#include <Glacier/Physics/Fysix/ZWaterBox.h>


namespace Glacier
{
    void Silevelcontrol__Missioncompleted()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Missionfailed(const char* reason)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Characterkilled(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Characterharmed(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Completeobjective(int objectiveId)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Completeobjectivenotickoffmark(int objectiveId)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Uncompleteobjective(int objectiveId)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Hideobjective(int objectiveId)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Unhideobjective(int objectiveId)
    {
        // TODO: Finish me
    }

    bool Silevelcontrol__Isobjectivecompleted(int objectiveId)
    {
        // TODO: Finish me
        return false;
    }

    void Silevelcontrol__Changeobjectivetext(int objectiveId, const char* text)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Clearchangedobjectivetext(int objectiveId)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Changeobjectivepath(int objectiveId, const char* path)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Clearchangedobjectivepath(int objectiveId)
    {
        // TODO: Finish me
    }

    ZREF Silevelcontrol__Getlevelcontrol()
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Silevelcontrol__Gethitman()
    {
        // TODO: Finish me
        return 0;
    }

    bool Silevelcontrol__Ishitmanrecognizable()
    {
        // TODO: Finish me
        return false;
    }

    void Silevelcontrol__Killhitman()
    {
        // TODO: Finish me
    }

    int Silevelcontrol__Getnotoriety()
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Silevelcontrol__Getcoverlist()
    {
        // TODO: Finish me
        return 0;
    }

    bool Silevelcontrol__Start_Actorinvestigatehitman(ZREF rInspector)
    {
        // TODO: Finish me
        return false;
    }

    void Silevelcontrol__End_Actorinvestigatehitman(ZREF rInspector)
    {
        // TODO: Finish me
    }

    Hitman::BloodMoney::SDifficultySettings Silevelcontrol__Getdifficultysettings()
    {
        // TODO: Finish me
        return {};
    }

    int Silevelcontrol__Getdifficulty()
    {
        // TODO: Finish me
        return 0;
    }

    bool Silevelcontrol__Ishitmantrespassing()
    {
        // TODO: Finish me
        return false;
    }

    bool Silevelcontrol__Ishitmaninoutsidelocation()
    {
        // TODO: Finish me
        return false;
    }

    bool Silevelcontrol__Iscontainerallowedhere(ZREF rContainer)
    {
        // TODO: Finish me
        return false;
    }

    ZREF Silevelcontrol__Gethumanshieldtarget()
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Silevelcontrol__Gethitmanhidecloset()
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Silevelcontrol__Getcurrentvictim()
    {
        // TODO: Finish me
        return 0;
    }

    bool Silevelcontrol__Ishitmanincloset(ZREF rCloset)
    {
        // TODO: Finish me
        return false;
    }

    bool Silevelcontrol__Killhitmanincloset()
    {
        // TODO: Finish me
        return false;
    }

    ZREF Silevelcontrol__Getdoorpickedbyhm()
    {
        // TODO: Finish me
        return 0;
    }

    bool Silevelcontrol__Ishitmanlockpicking()
    {
        // TODO: Finish me
        return false;
    }

    bool Silevelcontrol__Ishitmanloookingthroughkeyhole()
    {
        // TODO: Finish me
        return false;
    }

    bool Silevelcontrol__Ishitmanthrowingstuff()
    {
        // TODO: Finish me
        return false;
    }

    bool Silevelcontrol__Subscribehitmanchangeclothes(ZREF rSubscriber)
    {
        // TODO: Finish me
        return false;
    }

    ZREF Silevelcontrol__Getcurrenthitmansuit()
    {
        // TODO: Finish me
        return 0;
    }

    bool Silevelcontrol__Ishitmanwearinghitmansuit()
    {
        // TODO: Finish me
        return false;
    }

    ZREF Silevelcontrol__Getcurrenthitmanlefthanditem()
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Silevelcontrol__Getcurrenthitmanweapontemplate()
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Silevelcontrol__Getactorfriskinghm()
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Silevelcontrol__Setactorfriskinghm(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    Hitman::BloodMoney::EFriskOp Silevelcontrol__Getfriskoperation()
    {
        // TODO: Finish me
        return Hitman::BloodMoney::eeFriskOp_Attack;
    }

    Hitman::BloodMoney::EFriskOp Silevelcontrol__Setfriskoperation(Hitman::BloodMoney::EFriskOp operation)
    {
        // TODO: Finish me
        return Hitman::BloodMoney::eeFriskOp_Attack;
    }

    int Silevelcontrol__Getcurrentzone()
    {
        // TODO: Finish me
        return 0;
    }

    int Silevelcontrol__Starteventcamera(ZREF rAttachGeom, eEventCameraType cameraType, ZREF rLookAtGeom, eEventCameraPosition position)
    {
        // TODO: Finish me
        return 0;
    }

    int Silevelcontrol__Starteventcameralookatgeom(ZREF rLookAtGeom, ZREF rCameraGeom, eEventCameraPosition position)
    {
        // TODO: Finish me
        return 0;
    }

    int Silevelcontrol__Starteventcamerawithcameraboneanim(ZREF rCameraGeom, int animationIndex, int position)
    {
        // TODO: Finish me
        return 0;
    }

    int Silevelcontrol__Endeventcamera(int eventCameraId)
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Silevelcontrol__Getguardsfriskbox(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    bool Silevelcontrol__Hashmweaponinhand()
    {
        // TODO: Finish me
        return false;
    }

    void Silevelcontrol__Incfriskfails()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Incheadshots()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Inctargetspoisoned()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Incsedates()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Incimpersonations()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Incpusheddeath()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Inctargetssniped()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Incbodiesfound(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Incsedatedbodiesfound()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Incbloodtrailsfound()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Incclosecombatkill()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Incaccidentkill(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Incaccidentkillignoredamage(ZREF rActor)
    {
        // TODO: Finish me
    }

    bool Silevelcontrol__Setchangeclothesallowed(bool allowed)
    {
        // TODO: Finish me
        return false;
    }

    void Silevelcontrol__Caughtontape()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Tapestolen()
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Weaponused(ZREF rActor, ZREF rWeapon)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Settargetnum(ZREF rActor, int targetNum)
    {
        // TODO: Finish me
    }

    ZREF Silevelcontrol__Gethminventoryitem(const char* itemType)
    {
        // TODO: Finish me
        return 0;
    }

    bool Silevelcontrol__Isitemtemplateinhminventory(ZREF rTemplate)
    {
        // TODO: Finish me
        return false;
    }

    bool Silevelcontrol__Isiteminhminventory(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    void Silevelcontrol__Copyhmtoref(ZREF rRef)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Audioaddpauseobject(ZREF rObject)
    {
        // TODO: Finish me
    }

    int Silevelcontrol__Starteventmusic(int musicId)
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Silevelcontrol__Createsecuritydevicefromtemplate()
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Silevelcontrol__Getsuitcaseinhitmanshand()
    {
        // TODO: Finish me
        return 0;
    }

    bool Silevelcontrol__Isdooropen(ZREF rDoor)
    {
        // TODO: Finish me
        return false;
    }

    bool Silevelcontrol__Isdoorlocked(ZREF rDoor)
    {
        // TODO: Finish me
        return false;
    }

    void Silevelcontrol__Getclosestposter(ZREF rReference, ZREF& rPoster, v3& position)
    {
        // TODO: Finish me
    }

    void Silevelcontrol__Getreachablehitmanpfpos(v3& position)
    {
        // TODO: Finish me
    }

    bool Silevelcontrol__Getclosestexitposforclimbinghitman(v3 position, v3& exitPosition)
    {
        // TODO: Finish me
        return false;
    }

    bool Silevelcontrol__Ishitmaninreachableposition()
    {
        // TODO: Finish me
        return false;
    }

    bool Silevelcontrol__Ishitmaninsidepathfinder()
    {
        // TODO: Finish me
        return false;
    }

    void Silevelcontrol__Forcecloselasthitmandoor()
    {
        // TODO: Finish me
    }

    bool Silevelcontrol__Ishitmaninelevator()
    {
        // TODO: Finish me
        return false;
    }

    bool Silevelcontrol__Ishitmantopofelevator()
    {
        // TODO: Finish me
        return false;
    }

    void Silevelcontrol__Showonmap(ZREF rActor, bool show)
    {
        // TODO: Finish me
    }

    bool Silevelcontrol__Isactorinwaterbox(ZREF rActor)
    {
        auto* pGeom = ref_cast<ZGEOM>(rActor);
        if (!pGeom)
            return false;

        return ZWaterBoxManager::Instance()[pGeom] != nullptr;
    }

    void Silevelcontrol__Remove_Existing_Eventcam()
    {
        // TODO: Finish me
    }
}
