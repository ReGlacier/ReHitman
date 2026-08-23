#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct ZRenderEntryGeom;
    struct ZRenderView;

    struct  ZCmdList
    {
        // types
        enum CMD : int32_t
        {
            CMD_SET_VIEW = 0x0,
            CMD_VIEWSPACE_SETUP = 0x1,
            CMD_VIEWPORT_SETUP = 0x2,
            CMD_VIEWPORT_CLEAR = 0x3,
            CMD_LIGHT_SETUP = 0x4,
            CMD_LIGHT_SETUP_2 = 0x5,
            CMD_LIGHT_CAST_SHADOWS = 0x6,
            CMD_SHADOW_SETUP = 0x7,
            CMD_DROP_SHADOW_SETUP = 0x8,
            CMD_SHADOW_CASTERS_START = 0x9,
            CMD_SHADOW_CASTERS_START_2 = 0xA,
            CMD_SHADOW_RECEIVERS_START = 0xB,
            CMD_SHADOW_END = 0xC,
            CMD_CLEAR_AND_BEGIN_ALPHABLEND_MAX = 0xD,
            CMD_BEGIN_ALPHABLEND_MAX = 0xE,
            CMD_END_ALPHABLEND_MAX = 0xF,
            CMD_APPLY_ALPHABLEND_MAX = 0x10,
            CMD_SCISSOR_SETUP = 0x11,
            CMD_OBJECT_DRAW = 0x12,
            CMD_APPLY_POSTFILTER = 0x13,
            CMD_MIRROR_BEGIN = 0x14,
            CMD_MIRROR_RENDER_REFLECTORS = 0x15,
            CMD_MIRROR_RENDER_REFLECTED = 0x16,
            CMD_MIRROR_RENDER_ZHOLE = 0x17,
            CMD_MIRROR_END = 0x18,
            CMD_OPTIMIZED_MIRROR_CLEAR_ZBUFFER = 0x19,
            CMD_OPTIMIZED_MIRROR_RENDER_REFLECTORS = 0x1A,
            CMD_OPTIMIZED_MIRROR_SET_STENCIL_REF = 0x1B,
            CMD_OPTIMIZED_MIRROR_RENDER_REFLECTED = 0x1C,
            CMD_OPTIMIZED_MIRROR_END = 0x1D,
            CMD_CULL_NORMAL = 0x1E,
            CMD_CULL_REVERSED = 0x1F,
            CMD_PUSH_SURFACE = 0x20,
            CMD_POP_SURFACE = 0x21,
            CMD_SET_SURFACE = 0x22,
            CMD_CLEAR_SURFACE = 0x23,
            CMD_COPY_SURFACE = 0x24,
            CMD_FILL_SURFACE = 0x25,
            CMD_SET_COLOR_WRITE_MASK = 0x26,
            CMD_SET_Z_WRITE_ENABLE = 0x27,
            CMD_LIGHT_BOUNDS = 0x28,
            CMD_BLUR_SURFACE = 0x29,
            CMD_TEXTURE_PROJECTION_INDEX = 0x2A,
            CMD_RENDER_FULLSCREEN_QUAD = 0x2B,
            CMD_SET_PROJECTION = 0x2C,
            CMD_PUSH_PROJECTION = 0x2D,
            CMD_POP_PROJECTION = 0x2E,
            CMD_SET_FIRST_PERSON_BONES = 0x2F,
            CMD_SPRITE_MANAGER_BEGIN = 0x30,
            CMD_OBJECT_ADD_TO_TRANS_MGR = 0x31,
            CMD_SPRITE_MANAGER_DRAW = 0x32,
            CMD_SPRITE_MANAGER_END = 0x33,
            CMD_DUPEZBUFFER = 0x34,
            CMD_RETURNTOFIXEDPIPELINE = 0x35,
            CMD_SEEMS_FIXUP = 0x36,
            CMD_RESOLVE_CUBEMAP = 0x37,
            CMD_RESOLVE_SURFACE = 0x38,
            CMD_RESTORE_SURFACE = 0x39,
            CMD_TILING_BEGIN = 0x3A,
            CMD_TILING_END = 0x3B,
            CMD_Z_PREPASS_BEGIN = 0x3C,
            CMD_Z_PREPASS_END = 0x3D,
            CMD_SHADOW_ENABLE = 0x3E,
            CMD_RENDER_TO_TEXTURE = 0x3F,
            CMD_DRAW_BLOB_SHADOWS = 0x40,
            CMD_DRAW_BREAKABLES = 0x41,
            CMD_INSERT_RSX_CALLBACK = 0x42,
            CMD_HUD_PUSH_MARKER = 0x43,
            CMD_HUD_POP_MARKER = 0x44,
            CMD_SETANTIALIASINGCONTROL = 0x45,
            CMD_DOWNSAMPLE_SURFACE = 0x46,
            CMD_OBJECT_DRAW_OPT = 0x47,
            CMD_NOP = 0x48,
        };

        struct ZCmd
        {
            // methods
            void* AddData(uint32_t lDataSize);
            void* AddData(void* pData, uint32_t lDataSize);
            ZCmd* Next() const;

            // members
            ZCmdList::CMD m_lType;
            ZRenderEntryGeom* m_pRenderEntryGeom;
            ZCmdList* m_pCmdList;
            ZRenderView* m_pRenderView;
            uint32_t m_lLayer;
            uint32_t m_lNrObjects;
        };

        // methods
        ZCmd* Current() const;
        void NextCommand();

        // members
        uint32_t m_lPreTransactionNrCmds;
        char *m_pPreTransactionCurrent;
        uint32_t m_lNrCmds;
        char *m_pCurrent;
        char *m_pBufferEnd;
        char *m_Buffer;
    };
    RE_VERIFY_SIZE(ZCmdList, 0x18);
}
