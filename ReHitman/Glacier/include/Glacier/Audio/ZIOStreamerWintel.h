#pragma once

#include <Glacier/Audio/ZIOStreamer.h>

namespace Glacier
{
    class ZIOStreamerWintel : public ZIOStreamer
    {
    public:
        bool Create(int _numBlocks, int _blockSize, uint32_t _maxLatency) override;
        void Destroy() override;
        uint32_t GetFileHandle(const char* _fileName) override;
        bool Busy() override;
        void IssueRead() override;

        void* m_hThread;
        void* m_hIOEvent;
        void* m_hShutdownEvent;
    };

    RE_VERIFY_OFFSET(ZIOStreamerWintel, m_hThread, 0x23C);
    RE_VERIFY_OFFSET(ZIOStreamerWintel, m_hIOEvent, 0x240);
    RE_VERIFY_OFFSET(ZIOStreamerWintel, m_hShutdownEvent, 0x244);
    RE_VERIFY_SIZE(ZIOStreamerWintel, 0x248);
}
