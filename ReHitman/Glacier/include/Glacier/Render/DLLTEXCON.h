#pragma once

#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class DLLTEXCON
    {
    public:
        virtual ~DLLTEXCON() = default;
        virtual ZTextureD3D* GetTexture(uint32_t lTextureId, uint32_t lFrameId) = 0;
    };
}