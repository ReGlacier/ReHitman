#pragma once

#include <cstdint>


namespace Glacier
{
    // Flags for _ScriptState::m_Flags.
    static constexpr uint16_t ZSC_ASYNC_RESUME_CLEAR_MASK = 0x0003u;
    static constexpr uint16_t ZSC_LOCAL_ASYNC_BLOCK_MASK = 0x0005u;
    static constexpr uint16_t ZSC_ALIEN_ASYNC_BLOCK_MASK = 0x0007u;
    static constexpr uint16_t ZSC_FLAG_ASYNC_ACTIVE = 0x0002u;
    static constexpr uint16_t ZSC_FLAG_ASYNC_WAITING = 0x0004u;
    static constexpr uint16_t ZSC_FLAG_HANDLING_MESSAGE = 0x0008u;
    static constexpr uint16_t ZSC_FLAG_ALIEN_CALL_ACTIVE = 0x0010u;
    static constexpr uint16_t ZSC_FLAG_SKIP_MESSAGE_QUEUE = 0x0020u;
    static constexpr uint16_t ZSC_FLAG_CLEAR_AFTER_ENTRY = 0x0040u;
    static constexpr uint16_t ZSF_TERMINATE = 0x0080u;
    static constexpr uint16_t ZSC_FLAG_SUPPRESS_MESSAGE_COMMAND = 0x0100u;
    static constexpr uint16_t ZSC_CONTINUE_AFTER_SLEEP_MASK = 0x0600u;
    static constexpr uint16_t ZSC_FLAG_ALIEN_ASYNC_UNLINK = 0x0800u;
    static constexpr uint16_t ZSC_MESSAGE_QUEUE_COUNT_MASK = 0xF000u;
}
