#pragma once

#include <cstdint>


namespace Glacier
{
    // Masks
    static constexpr uint32_t ZCCOLIMASK = 0xFFu; // Weird, but approved in PS2 assembly
    static constexpr uint32_t ZCINACTIVE = 0x400u; // Used in combo ZCHIDDEN|ZCINACTIVE|ZCOWNERDRAW|ZCINVISIBLE
    static constexpr uint32_t ZCHIDDEN   = 0x800u; // Approved by ZBaseGeom::Hide
    static constexpr uint32_t ZCINVISIBLE = 0x2000u; // Approved by ZBaseGeom::Invisible
    static constexpr uint32_t ZCROOMASSIGN = 0x4000u; // Approved by ZBaseGeom::AutoAssignToRooms
    static constexpr uint32_t ZCCHKLIGHT = 0x8000u; // PS2
    static constexpr uint32_t ZCBOUNDSDIRTY = 0x100000u; // By myself
    static constexpr uint32_t ZCDYNAMIC = 0x40000u; // Approved by ZBaseGeom::AttachToDynamicContainer
    static constexpr uint32_t ZCRENDERATTACHED = 0x400000; // K&L 2
    static constexpr uint32_t ZCTEMPDRAW = 0x10000000; // K&L 2
    static constexpr uint32_t ZCOWNERDRAW = 0x200000u; // Used in old render & bone modify base?
    static constexpr uint32_t ZCNONRUNTIME = 0x8000000u; // K&L 2 from ZBaseGeom::SetControl
    static constexpr uint32_t ZCHASMOVED = 0x20000000u; /// Approved by ZEngineGeomControl::UpdateMovedGeoms
    static constexpr uint32_t ZCHASDYNAMICPARENT = 0x40000000u; // Approved by ZBaseGeom::GetDynamicParentPtr(
}