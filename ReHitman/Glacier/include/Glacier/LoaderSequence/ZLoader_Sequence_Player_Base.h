#pragma once

#include <Glacier/ReGlacier.h>

namespace Glacier
{
    // Abstract interface of the loading-sequence player.
    // PC vtable 0x00762974: deleting destructor + 5 _purecall slots.
    // Original source: engine\drawing\loader_sequence_player_base.cpp (XBOX_KL1 PDB).
    //
    // ZLoader_Sequence_Player (ZLoader_Sequence_Player.h) is the static facade
    // over this interface: it owns m_pLoader_Sequence_Player + m_pCreate_Func
    // and exposes Begin/Set_Progress/End (PC 0x4736B0/0x473620/0x473670).
    class ZLoader_Sequence_Player_Base
    {
    public:
        // vtbl
        virtual ~ZLoader_Sequence_Player_Base();                        // PC 0x473640 (complete), 0x473650 (deleting)
        virtual void Start() = 0;                                        // PC vtable slot 1
        virtual void Finish() = 0;                                       // PC vtable slot 2
        virtual void Progress(float fValue) = 0;                         // PC vtable slot 3
        virtual void Disable_Render() = 0;                               // PC vtable slot 4
        virtual void Enable_Render() = 0;                                // PC vtable slot 5
    };
    RE_VERIFY_SIZE(ZLoader_Sequence_Player_Base, 0x4); // XBOX_KL1 PDB
}
