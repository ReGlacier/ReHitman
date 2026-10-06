#include <Glacier/LoaderSequence/ZLoader_Sequence_Player.h>

namespace Glacier
{
    STATIC_CLASS_VAR_IMPL(ZLoader_Sequence_Player, ZLoader_Sequence_Player::TCreate_Func, m_pCreate_Func, 0x008EC138, nullptr);
    STATIC_CLASS_VAR_IMPL(ZLoader_Sequence_Player, ZLoader_Sequence_Player_Base*, m_pLoader_Sequence_Player, 0x008EC13C, nullptr);

    // PC 0x4736B0. Tears down any previous player, then creates and starts the
    // platform player through the registered factory.
    void ZLoader_Sequence_Player::Begin()
    {
        if (m_pLoader_Sequence_Player)
        {
            m_pLoader_Sequence_Player->Finish();
            if (m_pLoader_Sequence_Player)
            {
                // PC: deleting destructor call (vtable slot 0 with delete flag 1).
                ZUniMemory::Delete(m_pLoader_Sequence_Player);
            }
            m_pLoader_Sequence_Player = nullptr;
        }

        if (m_pCreate_Func)
            m_pLoader_Sequence_Player = m_pCreate_Func();

        if (m_pLoader_Sequence_Player)
            m_pLoader_Sequence_Player->Start();
    }

    // PC 0x473620.
    void ZLoader_Sequence_Player::Set_Progress(float fValue)
    {
        if (m_pLoader_Sequence_Player)
            m_pLoader_Sequence_Player->Progress(fValue);
    }

    // PC 0x473670. The doubled Finish()/null-checks are faithful to the original
    // machine code (the static is reloaded after every virtual call).
    void ZLoader_Sequence_Player::End()
    {
        if (m_pLoader_Sequence_Player)
        {
            m_pLoader_Sequence_Player->Finish();
            if (m_pLoader_Sequence_Player)
            {
                m_pLoader_Sequence_Player->Finish();
                if (m_pLoader_Sequence_Player)
                {
                    // PC: deleting destructor call (delete flag 1).
                    ZUniMemory::Delete(m_pLoader_Sequence_Player);
                }
                m_pLoader_Sequence_Player = nullptr;
            }
        }
    }
}
