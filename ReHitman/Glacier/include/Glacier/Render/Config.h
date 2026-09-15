#pragma once

#include <cstdint>


namespace Glacier
{
    /*
     * MiniNinjas - 512
     * K&L2 - 6144
     * HBM (iOS) - 1024
     * PC - ?
     *
     * In ZRenderEntryLists::GetList we have formula
     * 0x804 * lType + this + 0xC
     * ZStackArray<CAP, T> has view
     *   + 0x0 - size (4 bytes)
     *   + 0x4 - array
     * 0x804 - generic offset multiplied by list index
     * So in general, 0x4 - size, 0x800 - size in bytes of ALL elements in array
     * 0x800 / sizeof(ptr) = 0x200 (0x800/0x4)
     * 0x200 - count of elements = 512
     * So PC is 512
     */
    static constexpr uint32_t ELEMENTS_IN_RENDER_ENTRY_LIST_COUNT = 512;
}
