#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/ZMessageResolver.h>

namespace Glacier
{
    // Engine-internal action type ids, from the PS2 build's symbol dump
    // (Z:\code\hitman3\hitman3_ps2\startup.cpp). The numeric values are shared with
    // the PC build's ZAction::m_eType / Initialize() / FindAction() arguments.
    enum EActionType {
        AT_PICKUP = 0,
        AT_BUTTON = 1,
        AT_OPENDOOR = 2,
        AT_CLOSEDOOR = 3,
        AT_CLOTHES = 4,
        AT_DIALOG = 5,
        AT_GENERIC = 6,
        AT_PLACEITEM = 7,
        AT_ALWAYSINRANGE = 8,
        AT_DRAG = 9,
        AT_PICKDOOR = 10,
        AT_PEEKHOLE = 11,
        AT_DROPBODY = 12,
        AT_CLIMBWINDOW = 13,
        AT_CLIMBWALL = 14,
        AT_HIDECLOSET = 15,
        AT_GRAB = 16,
        AT_RELEASE = 17,
        AT_PUTITEMELEVATOR = 18,
        AT_JUMPBALCONY = 19,
        AT_PLACEBOMB = 20,
        AT_ACTIONKILL = 21,
        AT_USESWITCH = 22,
        AT_HIDEBODYTABLE = 23,
        AT_OPENLID = 24,
        AT_CLOSELID = 25,
        AT_RETRIEVEITEM = 26,
        AT_PLACE_NOINIT = 27,
        AT_PICKUP_NOINIT = 28,
        AT_OPERATEDOOR = 29,
        AT_BODYCONTAINER = 30,
        AT_USEKEYCARD = 31,
        AT_CLIMBHATCH = 32,
        AT_STRANGELINELEVATOR = 33,
        AT_ELEVATORBUTTON = 34,
        AT_PLACEITEMTOPOS = 35,
        AT_HMUSEWEAPONSTORAGE = 36,
        AT_BREAKUTILBOX = 37,
        AT_USELIGHTSWITCH = 38,
        AT_USEFIREALARM = 39,
        AT_SURRENDWEAPON = 40,
        AT_STEALTAPE = 41,
        AT_LAST = 42,
    };

    class ZAction;
    using ActionArray = ZStackArray<32, ZAction*>;

    class ZAction : public CBaseEvent<ZGEOM>
    {
    public:
        /// static message resolvers (registered by the engine's global ctors in action.cpp)
        STATIC_CLASS_VAR(ZAction, ZMessageResolver, s_msgCharacterEnterRange); // PC unk_99CDAC "CharacterEnterRange"
        STATIC_CLASS_VAR(ZAction, ZMessageResolver, s_msgCharacterLeaveRange); // PC unk_99CDB8 "CharacterLeaveRange"
        STATIC_CLASS_VAR(ZAction, ZMessageResolver, s_msgRequestGreying); // PC unk_99CDE8 "MSG_REQUESTGRAYING"
        STATIC_CLASS_VAR(ZAction, ZMessageResolver, s_msgCanOperateObject); // PC unk_99CDF4 "CanOperateObject"
        STATIC_CLASS_VAR(ZAction, ZMessageResolver, s_msgRemove); // PC unk_99CE00 "ActionRemove"
        STATIC_CLASS_VAR(ZAction, ZMessageResolver, s_msgEnableAction); // PC unk_99CE0C "EnableAction"
        STATIC_CLASS_VAR(ZAction, ZMessageResolver, s_msgDisableAction); // PC unk_99CE18 "DisableAction"
        STATIC_CLASS_VAR(ZAction, ZMessageResolver, s_msgGetNearObjects); // PC unk_99CE24 "GetNearObjects"
        STATIC_CLASS_VAR(ZAction, ZMessageResolver, s_msgRemoveNamedAction); // PC unk_99CE30 "RemoveNamedAction"

        /// vftable
        virtual ~ZAction() override;
        virtual bool InRange(ZGEOM* geom); //Allowed to pass only ZPlayer or ZHitman3, other values will be ignored!
        virtual ZAction* FindAction(const char*, const char*, EActionType type, ZREF);
        virtual void Run(ZREF refToEntityAsArgument);
        virtual void RunMultiple(ZREF refToEntityAsArgument);
        virtual void RunFinished(ZGEOM*);
        virtual void ChangeNames(const char* names);
        virtual void SetType(EActionType type);
        virtual void SetMessage(ZMSGID);
        virtual void SetPriority(unsigned int prio);
        virtual void SafeDelete();
        virtual void Initialize(
            const char* szActionName, 
            const char* szOptionName, 
            EActionType eType, 
            ZMSGID msgMessage, 
            ZREF rReceiver, 
            int lPriority, 
            int lRange, 
            ZREF rItemTemplate);
        virtual void ActionFrameUpdate(ZGEOM*);

        /// api
        ZAction();
        ZAction** GetActionArray();
        void Show();
        void Hide();

        // custom API
        /**
         * @brief Allocate and register new action
         * @param pGeom pointer to ZGEOM instance on scene
         * @param psLocalizedActionName path in LOC file
         * @param psActionName action name when localization not available
         * @param actionType kind of action (see EActionType for details)
         * @param commandId 2-byte command id, it will be sent to Command method of receiver instance
         * @param entityRef ref to receiver entity (must be inherited of ZEventBase)
         * @param unk0 unknown value, in most cases is zero
         * @param radius the radius accessibility of action
         * @return the created action, or nullptr if it could not be created
         */
        static ZAction* AddAction(
                ZGEOM* pGeom,
                const char* psLocalizedActionName,
                const char* psActionName,
                EActionType actionType,
                Glacier::ZMSGID commandId,
                Glacier::ZREF entityRef,
                int unk0,
                int radius);

    private:
        void SetNames(const char* szActionName, const char* szOriginalName);
        void ReleaseMem();
        void SendActionChange(void* pData);
        void UpdateObjectsInRange(ZGEOM* pGeom);
        void EnableActions(ZGEOM* pTarget);
        void DisableActions(ZGEOM* pTarget);

    public:
        /// data (total size if 0xFC, size of ZEventBase is 0x30)
        EActionType m_eType;
        ZMSGID m_msgMessage;
        ZREF m_rReceiver;
        int32_t m_lPriority;
        int32_t m_lRange;
        bool m_bHitmanReceiver;
        uint32_t m_lActionControl;
        bool m_bIsMaster;
        ZMSGID m_msgActionHide;
        ZMSGID m_msgActionShow;
        bool m_bIsInitialized;
        bool m_bIsItem;
        bool m_bColi2Enabled;
        int32_t m_lChanged;
        ZStackArray<2,unsigned int> m_NearObjects;
        ZRTString m_szOriginalString;
        ZRTString m_szActionName;
        ActionArray m_ActionArray;
        ActionArray* m_pActiveActionArray;
        ZREF UserData;
    };
    RE_VERIFY_SIZE(ZAction, 0xFC); // Verified
}