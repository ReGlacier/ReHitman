#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/EventBase/ZEventBase.h>

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

    class ZAction : public ZEventBase
    {
    public:
        /// vftable
        virtual bool InRange(ZGEOM* geom); //Allowed to pass only ZPlayer or ZHitman3, other values will be ignored!
        virtual ZAction* FindAction(const char*, const char*, EActionType type, Glacier::ZREF);
        virtual void Run(Glacier::ZREF refToEntityAsArgument);
        virtual void RunMultiple(Glacier::ZREF refToEntityAsArgument);
        virtual void RunFinished(Glacier::ZGEOM*);
        virtual void ChangeNames(const char** names);
        virtual void SetType(EActionType type);
        virtual void SetMessage(Glacier::ZMSGID);
        virtual void SetPriority(unsigned int prio);
        virtual void SafeDelete();
        virtual void Initialize(const char* name, const char* name2, EActionType type, Glacier::ZMSGID msg, unsigned int, int, int, int);
        virtual void ActionFrameUpdate(Glacier::ZGEOM*);

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
        EActionType m_type; //+0x30
        Glacier::ZMSGID m_message; //+0x34
        unsigned short m_field36; //+0x36
        Glacier::ZREF m_playerREF; //+0x38
        int field_3C;
        int field_40;
        int field_44;
        int field_48;  // Flags, |= 1u - invisible
        unsigned short field_4C;
        Glacier::ZMSGID m_actionHideMSG;
        Glacier::ZMSGID m_actionShowMSG;
        unsigned short field_52;
        int field_54;
        int field_58;
        int field_5C;
        int field_60;
        int field_64;
        int field_68;
        int field_6C;
        int field_70;
        int field_74;
        int field_78;
        int field_7C;
        int field_80;
        int field_84;
        int field_88;
        int field_8C;
        int field_90;
        int field_94;
        int field_98;
        int field_9C;
        int field_A0;
        int field_A4;
        int field_A8;
        int field_AC;
        int field_B0;
        int field_B4;
        int field_B8;
        int field_BC;
        int field_C0;
        int field_C4;
        int field_C8;
        int field_CC;
        int field_D0;
        int field_D4;
        int field_D8;
        int field_DC;
        int field_E0;
        int field_E4;
        int field_E8;
        int field_EC;
        int field_F0;
        int field_F4;
        int field_F8;
    };

    static_assert(offsetof(ZAction, m_type) == 0x30, "ZAction| Bad offset of field m_field30");
}