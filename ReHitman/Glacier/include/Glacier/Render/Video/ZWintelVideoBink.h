#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Video/ZBaseVideo.h>
#include <BinkVideoInterface.h>

namespace Glacier
{
    class ZWintelVideoBink final : public ZBaseVideo
    {
    public:
        ~ZWintelVideoBink() override;
        bool EndOfFile() override;
        void SetFileName(const char* pszFileName) override;
        void Init() override;
        void OnBeginPlay() override;
        void OnEndPlay() override;
        void ShowNextFrame() override;
        void SetSize(uint32_t lWidth, uint32_t lHeight) override;
        bool IsLastFrame() override;

        ZWintelVideoBink();
        static ZWintelVideoBink* BuildInstance();

    private:
        BinkVideoInterface::HBINK m_pBink;
        uint32_t m_lLastFrame;
    };

    RE_VERIFY_SIZE(ZWintelVideoBink, 0x1D0);
}
