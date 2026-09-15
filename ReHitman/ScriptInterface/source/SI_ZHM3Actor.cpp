#include <SI/SI_ZHM3Actor.h>

namespace Glacier
{
    void Zhm3Actor__Setblinking(ZREF rActor, bool disabled)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Initmapicon(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Forceshowmapicon(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Die(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Initializehitpoints(ZREF rActor, int hitPoints)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Sethitpoints(ZREF rActor, float hitPoints)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Setcharactertype(ZREF rActor, Hitman::BloodMoney::eCharacterType characterType)
    {
        // TODO: Finish me
    }

    Hitman::BloodMoney::eCharacterType Zhm3Actor__Getcharactertype(ZREF rActor, ZREF rDefaultActor)
    {
        // TODO: Finish me
        return Hitman::BloodMoney::eCharacterType_Enemy;
    }

    void Zhm3Actor__Changemapiconandcharactertype(ZREF rActor, Hitman::BloodMoney::eCharacterType characterType, bool forceDeadIcon)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Removefrommap(ZREF rActor)
    {
        // TODO: Finish me
    }

    float Zhm3Actor__Gethitpoints(ZREF rActor)
    {
        // TODO: Finish me
        return 0.0f;
    }

    float Zhm3Actor__Takedamage(ZREF rActor, float damage)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Zhm3Actor__Setleavebloodtrail(ZREF rActor, bool enabled)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Setvisionrangesandfovs(ZREF rActor, float closeRange, float closeFov, float farRange, float farFov)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Setvisionrangeandfov(ZREF rActor, float range, float fov)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Setmood(ZREF rActor, uint8_t mood, float intensity)
    {
        // TODO: Finish me
    }

    float Zhm3Actor__Gettension(ZREF rActor)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Zhm3Actor__Settension(ZREF rActor, Hitman::BloodMoney::eInterestLevel interestLevel)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Increasetension(ZREF rActor, Hitman::BloodMoney::eInterestLevel interestLevel, float amount)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Releasetension(ZREF rActor, Hitman::BloodMoney::eInterestLevel interestLevel)
    {
        // TODO: Finish me
    }

    uint8_t Zhm3Actor__Getmaxneed(ZREF rActor, float value)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Setneed(ZREF rActor, uint8_t need, float value)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Setneedtime(ZREF rActor, uint8_t need, float value)
    {
        // TODO: Finish me
    }

    float Zhm3Actor__Getneed(ZREF rActor, uint8_t need)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Zhm3Actor__Lockubanim(ZREF rActor, bool locked)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Enableubholdanims(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Disableubholdanims(ZREF rActor)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Enableubholdanim(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Isubanimlocked(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    int Zhm3Actor__Setscriptubanim(ZREF rActor, anim animId)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Resetscriptubanim(ZREF rActor)
    {
        // TODO: Finish me
    }

    int Zhm3Actor__Playscriptubanim(ZREF rActor, anim animId, bool force)
    {
        // TODO: Finish me
        return 0;
    }

    int Zhm3Actor__Playscriptubanimloop(ZREF rActor, anim animId, bool force)
    {
        // TODO: Finish me
        return 0;
    }

    int Zhm3Actor__Stopscriptubanimplay(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zhm3Actor__Hasrunningubanim(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Setweapontemplate(ZREF rActor, ZREF rWeaponTemplate)
    {
        // TODO: Finish me
    }

    ZREF Zhm3Actor__Getweapontemplate(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Setweapon(ZREF rActor, ZREF rWeapon)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Resetweapon(ZREF rActor)
    {
        // TODO: Finish me
    }

    ZREF Zhm3Actor__Getweapon(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Setammoinweapon(ZREF rActor, int ammo)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Pullweapon(ZREF rActor)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Ownsownweapon(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Isweapondrawn(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Currentweaponcanbeholstered(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Holsterweapon(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Isholdingitem(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Hasfreehand(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Islinkactionrunning(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Putiteminrightpocket(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Putiteminleftpocket(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Pullitemfromrightpocket(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Pullitemfromleftpocket(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Setisholdingshootableiteml(ZREF rActor, bool holding)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Setisholdingshootableitemr(ZREF rActor, bool holding)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Makeitemvisible(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Makeiteminvisible(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Makeactorvisible(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Makeactorinvisible(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Removedecal(ZREF rActor, int decalId)
    {
        // TODO: Finish me
    }

    int Zhm3Actor__Setvisibletypes(ZREF rActor, int visibleTypes)
    {
        // TODO: Finish me
        return 0;
    }

    int Zhm3Actor__Getvisibletypes(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Setseerblind(ZREF rActor, bool blind)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Disablevision(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Enablevision(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Disablepickupclothes(ZREF rActor, ZREF rClothes)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Enablepickupclothes(ZREF rActor, ZREF rClothes)
    {
        // TODO: Finish me
    }

    int Zhm3Actor__Getvisibleprojectileweapons(ZREF rActor, ZREF* outWeapons, int maxWeapons, ZREF rExcludeWeapon)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zhm3Actor__Isitemedible(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Isitemdrinkable(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    int Zhm3Actor__Getitemnumbitestotal(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return 0;
    }

    int Zhm3Actor__Getitemnumbites(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return 0;
    }

    int Zhm3Actor__Removeitembite(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Restoreitembites(ZREF rActor, ZREF rItem, int bites)
    {
        // TODO: Finish me
    }

    int Zhm3Actor__Getiteminfectiontype(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Aimweaponatgeom(ZREF rActor, ZREF rTarget, float blendTime)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Aimweaponatpos(ZREF rActor, v3 position, float blendTime)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Fireweapon(ZREF rActor, bool fire, float blendTime, bool unknown)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Playfireweaponsound(ZREF rActor, ZREF rWeapon)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__M12_Albinofireweapon(ZREF rActor, bool fire, float blendTime, bool unknown)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Triggerweaponpush(ZREF rActor, ZREF rWeapon)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Isaiminposition(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Isaimingattarget(ZREF rActor, ZREF rTarget, float tolerance, ZREF& rAimedTarget, v3& aimPosition)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Reloadweapon(ZREF rActor, bool instant)
    {
        // TODO: Finish me
        return false;
    }

    int Zhm3Actor__Numberofshotsleft(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zhm3Actor__Isbodyknownandunchanged(ZREF rActor, ZREF rBody)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Registerdeadbody(ZREF rActor, ZREF rBody)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Isobjectvisible(ZREF rActor, ZREF rObject)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Objectisinfront(ZREF rActor, ZREF rObject)
    {
        // TODO: Finish me
        return false;
    }

    float Zhm3Actor__Chkcolibeforeactor(ZREF rActor, float distance, float radius, v3& position)
    {
        // TODO: Finish me
        return 0.0f;
    }

    ZREF Zhm3Actor__Isitemininventory(ZREF rActor, const char* itemName)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zhm3Actor__Additemtoinventory(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Removeitemfrominventory(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Dumpinventorytoground(ZREF rActor)
    {
        // TODO: Finish me
    }

    ZREF Zhm3Actor__Undress(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zhm3Actor__Dress(ZREF rActor, ZREF rClothes)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Dialoglisten(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Dialogtalk(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Throwknifeatref(ZREF rActor, ZREF rKnife, ZREF rTarget)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Findsitdowngeom(ZREF rActor, v3& position, v3& direction, int& seatIndex, int flags, ZREF rIgnore)
    {
        // TODO: Finish me
        return false;
    }

    int Zhm3Actor__Sitdown(ZREF rActor, anim animId, bool instant)
    {
        // TODO: Finish me
        return 0;
    }

    int Zhm3Actor__Sitdowninstant(ZREF rActor, anim animId, bool instant)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zhm3Actor__Movechairifapplicable(ZREF rActor, anim animId, bool instant)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Movechair(ZREF rActor, anim animId, bool instant)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Situp(ZREF rActor)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Findliedowngeom(ZREF rActor, v3& position, v3& direction, bool allowOccupied)
    {
        // TODO: Finish me
        return false;
    }

    int Zhm3Actor__Liedown(ZREF rActor, bool instant)
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Zhm3Actor__Findlightswitch(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Togglelightswitch(ZREF rActor, ZREF rLightSwitch, bool on)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Getlightswitchstate(ZREF rActor, ZREF rLightSwitch)
    {
        // TODO: Finish me
        return false;
    }

    int Zhm3Actor__Getbodyposition(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Setbodyposition(ZREF rActor, int bodyPosition)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Setbodypositionunknown(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Setbodypositionnormal(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Sethmescort(ZREF rActor, ZREF rEscort)
    {
        // TODO: Finish me
    }

    ZREF Zhm3Actor__Gethmescort(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Playsound(ZREF rActor, eAudioEvent audioEvent)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Isweaponallowed(ZREF rActor, ZREF rWeapon)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Isweaponfoundathitman(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    ZREF Zhm3Actor__Transferhitmanweaponstoactor(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zhm3Actor__Isonback(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Setneardeath(ZREF rActor, bool nearDeath)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Dropbloodstain(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Activatebloodspurt(ZREF rActor, int bloodSpurt, v3& position)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Wantbloodstain(ZREF rActor, bool wantBloodStain)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Setwantbloodonhit(ZREF rActor, bool wantBloodOnHit)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Isactor(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Ishitmandraggingactor(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Iswearingclothes(ZREF rActor, ZREF rClothes)
    {
        // TODO: Finish me
        return false;
    }

    ZREF Zhm3Actor__Getoriginalsuit(ZREF rActor, ZREF rClothes)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zhm3Actor__Getflankingpos(ZREF rActor, ZREF rTarget, eDirection direction, v3& position)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Getbodycheckposdir(ZREF rActor, ZREF rBody, v3& position, v3& direction)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Getposonpath(ZREF rActor, ZREF rPath, float distance, v3& position)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Blowhitmandisguise(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Generateaudioevent(ZREF rActor, eAudioEvent audioEvent)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Playactorspeech(ZREF rActor, eAudioType audioType, eAudioEvent audioEvent)
    {
        // TODO: Finish me
    }

    int Zhm3Actor__Playresourceasspeech(ZREF rActor, int resourceId)
    {
        // TODO: Finish me
        return 0;
    }

    int Zhm3Actor__Playlocaleresourceasspeech(ZREF rActor, const char* resourceName)
    {
        // TODO: Finish me
        return 0;
    }

    int Zhm3Actor__Getlocalesoundresource(ZREF rActor, const char* resourceName)
    {
        // TODO: Finish me
        return 0;
    }

    int Zhm3Actor__Playrandomlocaleresourceasspeech(ZREF rActor, const char* resourceName)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zhm3Actor__Isspeechplaying(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Stopspeech(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Bite(ZREF rActor, int16_t animVariation, v3 from, v3 to, float blendIn, float blendOut)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Punch(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Stungunattack(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Activaterigidbody(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Userigidbodyonpush(ZREF rActor, bool enabled)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Useubragdoll(ZREF rActor, bool enabled)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Dragbody(ZREF rActor, ZREF rBody)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Actorbeingpushed(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Isactorbeingkilled(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Changeintobodybag(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Isbodybag(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Maybebodybagged(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Setmaybebodybagged(ZREF rActor, bool maybeBodyBagged)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Setignorehelprequests(ZREF rActor, bool ignoreHelpRequests)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Setissearching(ZREF rActor, bool isSearching)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Disabledrag(ZREF rActor, ZREF rBody)
    {
        // TODO: Finish me
    }

    int Zhm3Actor__Getnumberofavailableragdolls(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Setactorspecies(ZREF rActor, int actorSpecies)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Sendeventtoactorswithinbox(ZREF rActor, ZSC_EVENT event, ZREF rBox)
    {
        // TODO: Finish me
    }

    int Zhm3Actor__Playanimwithcamerabone(ZREF rActor, anim animId)
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Zhm3Actor__Findclosestcharactertype(ZREF rActor, int actorType, v3 position, float radius)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Disableragdollanddieanim(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Dummy(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Repairutilbox(ZREF rActor, ZREF rUtilBox)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Terminatecurrentlnkaction(ZREF rActor)
    {
        // TODO: Finish me
    }

    int Zhm3Actor__Getsecuritydeviceloopingsound(ZREF rActor, ZREF rSecurityDevice)
    {
        // TODO: Finish me
        return 0;
    }

    int Zhm3Actor__Getsecuritydeviceweaponfoundsound(ZREF rActor, ZREF rSecurityDevice)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Activateragdollnextframe(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Activateragdoll(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Hitragdoll(ZREF rActor, v3 position, v3 impulse, float force)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Ishitmanvisible(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Setignoredisguise(ZREF rActor, bool ignoreDisguise)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Isignoredisguise(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Canplayanimpf(ZREF rActor, anim animId, int flags)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Setposdiractor(ZREF rActor, v3& position, v3& direction)
    {
        // TODO: Finish me
    }

    float Zhm3Actor__Calculatestopdistancetoavoidobstacle(ZREF rActor)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Zhm3Actor__Setinterruptable(ZREF rActor, bool interruptable)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Positionlock(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Positionunlock(ZREF rActor)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Isanimubanim(ZREF rActor, anim animId)
    {
        // TODO: Finish me
        return false;
    }

    float Zhm3Actor__Getangletoactor2(ZREF rActor, ZREF rTarget, ZREF rReference)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Zhm3Actor__Addmouthemitter(ZREF rActor, ZREF rEmitter)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Removemouthemitter(ZREF rActor, ZREF rEmitter)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Onpickupitem(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Isenteringorleavingelevator(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Isinorusingelevator(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Isinoutsidelocation(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    ZREF Zhm3Actor__Isinelevator(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Zhm3Actor__Getelevatorhatchgeom(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    void Zhm3Actor__Broadcasttoothersinelevator(ZREF rActor, ZREF rElevator, ZSC_EVENT event)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Isaccidentkilled(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zhm3Actor__Ispointinsideelevatorarea(ZREF rActor, v3 position)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Setsuspectedsuit(ZREF rActor, ZREF rSuit)
    {
        // TODO: Finish me
    }

    bool Zhm3Actor__Ishitmanhidingbehindnextdoor(ZREF rActor, ZREF& rDoor, v3& position)
    {
        // TODO: Finish me
        return false;
    }

    void Zhm3Actor__Idlelookat(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
    }

    void Zhm3Actor__Setdisableragdollforshots(ZREF rActor, bool disable)
    {
        // TODO: Finish me
    }
}
