#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/EventBase/ZEventBase.h>

namespace Glacier
{
    enum class EActionType : int {
        Type1 = 6,
        PlaceAction = 7
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

        /// data (total size if 0xFC, size of ZEventBase is 0x30)
        EActionType m_type; //+0x30
        Glacier::ZMSGID m_message; //+0x34
        unsigned short m_field36; //+0x36
        Glacier::ZREF m_playerREF; //+0x38
        int field_3C;
        int field_40;
        int field_44;
        int field_48;
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