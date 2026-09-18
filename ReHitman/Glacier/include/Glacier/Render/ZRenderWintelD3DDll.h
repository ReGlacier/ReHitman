#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZRenderWintelDll.h>


namespace Glacier
{
    class ZRenderWintelD3DDll : public ZRenderWintelDll
    {
    public:
        // vtbl
        ~ZRenderWintelD3DDll() override;
        void Init() override;
        void End() override;
        ZRender* SetupWindow(void* pWindow) override;
        void CreatePrimControl() override;
        void InstallPrimBuffer(void* pPrimBuffer, uint32_t lPrimBufferSize) override;
        void InstallTextureBuffer(void* pTextureBuffer, uint32_t lTextureBufferSize) override;
        void InstallMaterialBuffer(void* pBuffer, uint32_t lBufferSize, uint32_t lAllocatedBufferSize) override;
        uint32_t ReserveTexture(uint32_t lWidth, uint32_t lHeight) override;
        void UpdateTexture(uint32_t lTextureId, const ZBitmap* pBitmap) override;

        // methods
        ZRenderWintelD3DDll();
    };
    RE_VERIFY_SIZE(ZRenderWintelD3DDll, 0xBC); // Verified PC alloc at CreateD3DDll
}
