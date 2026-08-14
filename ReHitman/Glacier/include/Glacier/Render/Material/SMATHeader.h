#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // Header of the MAT buffer. All offsets are relative to the beginning of the buffer.
    struct SMATHeader
    {
        uint32_t lClassTableOffset { 0u };    // Offset of the material classes table (SRMaterialProperties records)
        uint32_t lInstanceTableOffset { 0u }; // Offset of the material instances table (SRMaterialProperties records)
        uint32_t lReserved { 0u };            // Always zero
        uint32_t lUnknownTableOffset { 0u };  // Not used by ZRenderMaterialBufferD3D::CreateMaterials
    };
    RE_VERIFY_SIZE(SMATHeader, 0x10);
}
