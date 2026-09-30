#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/LoaderSequence/ZLoader_Sequence_Player_Base.h>

namespace Glacier
{
    // Static facade over ZLoader_Sequence_Player_Base. Platform modules register
    // a factory through m_pCreate_Func (PC static-init 0x73B220 installs
    // &ZLoader_Sequence_Wintel_D3D::Produce); Begin/End drive the player around
    // scene loads and Set_Progress feeds it load percentages.
    class ZLoader_Sequence_Player
    {
    public:
        // Platform player factory (STATIC_CLASS_VAR cannot carry an inline
        // function-pointer declarator, hence the typedef).
        typedef ZLoader_Sequence_Player_Base* (*TCreate_Func)();

        // static
        STATIC_CLASS_VAR(ZLoader_Sequence_Player, TCreate_Func, m_pCreate_Func);                                 // PC 0x008EC138
        STATIC_CLASS_VAR(ZLoader_Sequence_Player, ZLoader_Sequence_Player_Base*, m_pLoader_Sequence_Player);     // PC 0x008EC13C

        static void Begin();                 // rev @ 0x4736B0 (IDA name InitLoaderSequence)
        static void Set_Progress(float fValue); // rev @ 0x473620
        static void End();                   // rev @ 0x473670
    };
}
