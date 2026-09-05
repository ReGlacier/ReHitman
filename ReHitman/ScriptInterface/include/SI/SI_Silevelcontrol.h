#pragma once

#include <BloodMoney/Game/SDifficultySettings.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <Glacier/ScriptEngine/Common.h>
#include <Glacier/ZREF.h>

namespace Glacier
{
    void Silevelcontrol__Missioncompleted();
    void Silevelcontrol__Missionfailed(const char* reason);
    void Silevelcontrol__Characterkilled(ZREF rActor);
    void Silevelcontrol__Characterharmed(ZREF rActor);
    void Silevelcontrol__Completeobjective(int objectiveId);
    void Silevelcontrol__Completeobjectivenotickoffmark(int objectiveId);
    void Silevelcontrol__Uncompleteobjective(int objectiveId);
    void Silevelcontrol__Hideobjective(int objectiveId);
    void Silevelcontrol__Unhideobjective(int objectiveId);
    bool Silevelcontrol__Isobjectivecompleted(int objectiveId);
    void Silevelcontrol__Changeobjectivetext(int objectiveId, const char* text);
    void Silevelcontrol__Clearchangedobjectivetext(int objectiveId);
    void Silevelcontrol__Changeobjectivepath(int objectiveId, const char* path);
    void Silevelcontrol__Clearchangedobjectivepath(int objectiveId);
    ZREF Silevelcontrol__Getlevelcontrol();
    ZREF Silevelcontrol__Gethitman();
    bool Silevelcontrol__Ishitmanrecognizable();
    void Silevelcontrol__Killhitman();
    int Silevelcontrol__Getnotoriety();
    ZREF Silevelcontrol__Getcoverlist();
    bool Silevelcontrol__Start_Actorinvestigatehitman(ZREF rInspector);
    void Silevelcontrol__End_Actorinvestigatehitman(ZREF rInspector);
    Hitman::BloodMoney::SDifficultySettings Silevelcontrol__Getdifficultysettings();
    int Silevelcontrol__Getdifficulty();
    bool Silevelcontrol__Ishitmantrespassing();
    bool Silevelcontrol__Ishitmaninoutsidelocation();
    bool Silevelcontrol__Iscontainerallowedhere(ZREF rContainer);
    ZREF Silevelcontrol__Gethumanshieldtarget();
    ZREF Silevelcontrol__Gethitmanhidecloset();
    ZREF Silevelcontrol__Getcurrentvictim();
    bool Silevelcontrol__Ishitmanincloset(ZREF rCloset);
    bool Silevelcontrol__Killhitmanincloset();
    ZREF Silevelcontrol__Getdoorpickedbyhm();
    bool Silevelcontrol__Ishitmanlockpicking();
    bool Silevelcontrol__Ishitmanloookingthroughkeyhole();
    bool Silevelcontrol__Ishitmanthrowingstuff();
    bool Silevelcontrol__Subscribehitmanchangeclothes(ZREF rSubscriber);
    ZREF Silevelcontrol__Getcurrenthitmansuit();
    bool Silevelcontrol__Ishitmanwearinghitmansuit();
    ZREF Silevelcontrol__Getcurrenthitmanlefthanditem();
    ZREF Silevelcontrol__Getcurrenthitmanweapontemplate();
    ZREF Silevelcontrol__Getactorfriskinghm();
    ZREF Silevelcontrol__Setactorfriskinghm(ZREF rActor);
    Hitman::BloodMoney::EFriskOp Silevelcontrol__Getfriskoperation();
    Hitman::BloodMoney::EFriskOp Silevelcontrol__Setfriskoperation(Hitman::BloodMoney::EFriskOp operation);
    int Silevelcontrol__Getcurrentzone();
    int Silevelcontrol__Starteventcamera(ZREF rAttachGeom, eEventCameraType cameraType, ZREF rLookAtGeom, eEventCameraPosition position);
    int Silevelcontrol__Starteventcameralookatgeom(ZREF rLookAtGeom, ZREF rCameraGeom, eEventCameraPosition position);
    int Silevelcontrol__Starteventcamerawithcameraboneanim(ZREF rCameraGeom, int animationIndex, int position);
    int Silevelcontrol__Endeventcamera(int eventCameraId);
    ZREF Silevelcontrol__Getguardsfriskbox(ZREF rActor);
    bool Silevelcontrol__Hashmweaponinhand();
    void Silevelcontrol__Incfriskfails();
    void Silevelcontrol__Incheadshots();
    void Silevelcontrol__Inctargetspoisoned();
    void Silevelcontrol__Incsedates();
    void Silevelcontrol__Incimpersonations();
    void Silevelcontrol__Incpusheddeath();
    void Silevelcontrol__Inctargetssniped();
    void Silevelcontrol__Incbodiesfound(ZREF rActor);
    void Silevelcontrol__Incsedatedbodiesfound();
    void Silevelcontrol__Incbloodtrailsfound();
    void Silevelcontrol__Incclosecombatkill();
    void Silevelcontrol__Incaccidentkill(ZREF rActor);
    void Silevelcontrol__Incaccidentkillignoredamage(ZREF rActor);
    bool Silevelcontrol__Setchangeclothesallowed(bool allowed);
    void Silevelcontrol__Caughtontape();
    void Silevelcontrol__Tapestolen();
    void Silevelcontrol__Weaponused(ZREF rActor, ZREF rWeapon);
    void Silevelcontrol__Settargetnum(ZREF rActor, int targetNum);
    ZREF Silevelcontrol__Gethminventoryitem(const char* itemType);
    bool Silevelcontrol__Isitemtemplateinhminventory(ZREF rTemplate);
    bool Silevelcontrol__Isiteminhminventory(ZREF rItem);
    void Silevelcontrol__Copyhmtoref(ZREF rRef);
    void Silevelcontrol__Audioaddpauseobject(ZREF rObject);
    int Silevelcontrol__Starteventmusic(int musicId);
    ZREF Silevelcontrol__Createsecuritydevicefromtemplate();
    ZREF Silevelcontrol__Getsuitcaseinhitmanshand();
    bool Silevelcontrol__Isdooropen(ZREF rDoor);
    bool Silevelcontrol__Isdoorlocked(ZREF rDoor);
    void Silevelcontrol__Getclosestposter(ZREF rReference, ZREF& rPoster, v3& position);
    void Silevelcontrol__Getreachablehitmanpfpos(v3& position);
    bool Silevelcontrol__Getclosestexitposforclimbinghitman(v3 position, v3& exitPosition);
    bool Silevelcontrol__Ishitmaninreachableposition();
    bool Silevelcontrol__Ishitmaninsidepathfinder();
    void Silevelcontrol__Forcecloselasthitmandoor();
    bool Silevelcontrol__Ishitmaninelevator();
    bool Silevelcontrol__Ishitmantopofelevator();
    void Silevelcontrol__Showonmap(ZREF rActor, bool show);
    bool Silevelcontrol__Isactorinwaterbox(ZREF rActor);
    void Silevelcontrol__Remove_Existing_Eventcam();
}
