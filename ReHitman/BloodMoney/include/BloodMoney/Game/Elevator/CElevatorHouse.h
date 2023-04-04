#pragma once

#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/REFTAB.h>

namespace Hitman::BloodMoney {
    class CElevatorHouse : public Glacier::ZEventBase {
    public:
        // vftable (not changed)

        // custom structures
        struct TUnk10 {
            int m_field0;
            int m_field4;
            int m_field8;
            bool m_fieldC : 1;
            bool m_fieldD : 1;
            bool m_fieldE : 1;
            bool m_fieldF : 1;
        };

        enum EElevatorStatus : int {
            ELEVATOR_STATUS_STOPPED = 0,
            ELEVATOR_STATUS_MOVE_UP = 1,
            ELEVATOR_STATUS_MOVE_DOWN = 2,
            ELEVATOR_STATUS_PAUSED = 4,
            ELEVATOR_STATUS_REACHED_END_POINT = 5,
            // --- IDK ---
            ELEVATOR_STATUS_UNKNOWN_3 = 3,
            ELEVATOR_STATUS_UNKNOWN_7 = 7
        };

        /**
         * @command RequestElevatorInfo
         */
        struct ElevatorInfo {
            EElevatorStatus  m_status; // Value from m_field76C
            bool m_unk4; // sub_5C2370(this);
            bool m_unk5; // unused
            bool m_unk6; // unused
            bool m_unk7; // unused
            int  m_unk8; // unused
            Glacier::REFTAB* m_unkRefTab38;
            int  m_unkField84;
        };

        /**
         * @command RequestFloorDef
         */
        struct FloorDef {
            Glacier::Vector3 m_position;
            int pFloorDef; // sub_5C19D0(arg); (?)
        };

        enum EDoorStatus {
            DOOR_STATUS_INVALID = -1,
            DOOR_STATUS_1 = 1,
            DOOR_STATUS_2 = 2,
            DOOR_STATUS_3 = 3,
        };

        // Custom methods
        /**
         * @fn GetDoorStatusOfElevatorByREF
         * @param elevatorObjectREF - Glacier REF to ZGEOM object
         * @return door status != DOOR_STATUS_INVALID if elevator object valid and interface presented, otherwise DOOR_STATUS_INVALID
         */
        static EDoorStatus GetDoorStatusOfElevatorByREF(Glacier::ZREF elevatorObjectREF);

        // data (total size is 0x778)
        float m_field30;
        int m_field34;
        Glacier::REFTAB m_reftab38;
        Glacier::REFTAB m_reftab54;
        Glacier::ZMSGID m_MSG_70;
        Glacier::ZMSGID m_MSG_72;
        int m_field74;
        int m_field78;
        int m_field7c;
        int m_field80;
        int m_field84;
        int m_field88;
        int m_field8c;
        Glacier::REFTAB m_reftab90;
        int m_fieldAC;
        int m_fieldDB0_EvREF;
        int m_moveToREF;
        Glacier::Vector3 m_v3MoveToPos;
        int m_fieldc4;
        int m_requestElevator;
        int m_fieldcc;
        int m_fieldd0;
        int m_fieldd4;
        int m_fieldd8;
        int m_fielddc;
        int m_fielde0;
        int m_fielde4;
        int m_fielde8;
        int m_fieldec;
        int m_fieldf0;
        int m_fieldf4;
        int m_fieldf8;
        int m_fieldfc;
        int m_field100;
        int m_field104;
        int m_field108;
        int m_field10c;
        int m_field110;
        int m_field114;
        int m_field118;
        int m_field11c;
        int m_field120;
        int m_field124;
        int m_field128;
        int m_field12c;
        int m_field130;
        int m_field134;
        int m_field138;
        int m_field13c;
        int m_field140;
        int m_field144;
        int m_field148;
        int m_field14c;
        int m_field150;
        int m_field154;
        int m_field158;
        int m_field15c;
        int m_field160;
        int m_field164;
        int m_field168;
        int m_field16c;
        int m_field170;
        int m_field174;
        int m_field178;
        int m_field17c;
        int m_field180;
        int m_field184;
        int m_field188;
        int m_field18c;
        int m_field190;
        int m_field194;
        int m_field198;
        int m_field19c;
        int m_field1a0;
        int m_field1a4;
        int m_field1a8;
        int m_field1ac;
        int m_field1b0;
        int m_field1b4;
        int m_field1b8;
        int m_field1bc;
        int m_field1c0;
        int m_field1c4;
        int m_field1c8;
        int m_field1cc;
        int m_field1d0;
        int m_field1d4;
        int m_field1d8;
        int m_field1dc;
        int m_field1e0;
        int m_field1e4;
        int m_field1e8;
        int m_field1ec;
        int m_field1f0;
        int m_field1f4;
        int m_field1f8;
        int m_field1fc;
        int m_field200;
        int m_field204;
        int m_field208;
        int m_field20c;
        int m_field210;
        int m_field214;
        int m_field218;
        int m_field21c;
        int m_field220;
        int m_field224;
        int m_field228;
        int m_field22c;
        int m_field230;
        int m_field234;
        int m_field238;
        int m_field23c;
        int m_field240;
        int m_field244;
        int m_field248;
        int m_field24c;
        int m_field250;
        int m_field254;
        int m_field258;
        int m_field25c;
        int m_field260;
        int m_field264;
        int m_field268;
        int m_field26c;
        int m_field270;
        int m_field274;
        int m_field278;
        int m_field27c;
        int m_field280;
        int m_field284;
        int m_field288;
        int m_field28c;
        int m_field290;
        int m_field294;
        int m_field298;
        int m_field29c;
        int m_field2a0;
        int m_field2a4;
        int m_field2a8;
        int m_field2ac;
        int m_field2b0;
        int m_field2b4;
        int m_field2b8;
        int m_field2bc;
        int m_field2c0;
        int m_field2c4;
        int m_field2c8;
        int m_field2cc;
        int m_field2d0;
        int m_field2d4;
        int m_field2d8;
        int m_field2dc;
        int m_field2e0;
        int m_field2e4;
        int m_field2e8;
        int m_field2ec;
        int m_field2f0;
        int m_field2f4;
        int m_field2f8;
        int m_field2fc;
        int m_field300;
        int m_field304;
        int m_field308;
        int m_field30c;
        int m_field310;
        int m_field314;
        int m_field318;
        int m_field31c;
        int m_field320;
        int m_field324;
        int m_field328;
        int m_field32c;
        int m_field330;
        int m_field334;
        int m_field338;
        int m_field33c;
        int m_field340;
        int m_field344;
        int m_field348;
        CElevatorHouse::TUnk10 m_unkArr34C[64];
        int m_field74c;
        int m_pivotDelta;
        int m_field754;
        int m_elevBound;
        int m_field75c;
        int m_field760;
        int m_field764;
        int m_field768;
        int m_currentElevatorStatus;
        int m_prevElevatorStatus;
        int m_field774;
    };
}