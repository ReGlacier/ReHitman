#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/ZSTL/ZRTStringObject.h>

namespace Glacier
{
    // 0 - N/A
    // 1 - N/A
    // 2 - Open Door
    // 3 - Close Door
    // 4 - N/A
    // 5 - Talk
    // 6 - Our custom action in "bottom" actions slot ("E")
    // 7 - Same to 6
    // 8 - N/A
    // 9 - Drag Body
    // 10 - N/A
    // 11 - N/A
    enum EActionType {
        OPEN_DOOR = 2,
        CLOSE_DOOR = 3,
        TALK = 5,
        CUSTOM = 6,
        CUSTOM2 = 7,
        ALWAYS_IN_RANGE = 8,
        DRAG_BODY = 9,
        STEAL_TAPE = 0x29
    };

    class ZAction;
    using ActionArray = ZStackArray<32, ZAction*>;

    class ZAction : public CBaseEvent<ZGEOM>
    {
    public:
        /// vftable
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
         * @return action ref
         */
        static int AddAction(
                ZGEOM* pGeom,
                const char* psLocalizedActionName,
                const char* psActionName,
                EActionType actionType,
                Glacier::ZMSGID commandId,
                Glacier::ZREF entityRef,
                int unk0,
                int radius);

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