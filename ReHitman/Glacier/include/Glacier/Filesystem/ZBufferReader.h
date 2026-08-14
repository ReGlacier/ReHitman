#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Filesystem/Fwd.h>
#include <Glacier/Filesystem/IBuffer.h>


namespace Glacier
{
    class ZBufferReader : public IBuffer
    {
    public:
        // vtbl
        ~ZBufferReader() override;
        const void* GetData(uint32_t lOffset) override;
        virtual void InstallBuffer(void* pBuffer, uint32_t lBufferSize, uint32_t lBufferSizeAllocated);
        virtual void FreeBuffer();
        virtual void AllocateResources();
        virtual void FreeResources();

        // methods
        ZBufferReader();

        // members
        char* m_pBuffer{nullptr}; // +0x4
        uint32_t m_lBufferSize{0}; // +0x8
        uint32_t m_lBufferSizeAllocated{0}; // +0xC
        uint32_t m_lBufferEnd{0}; // +0x10
        ZBufferPacker* m_pPacker{nullptr}; // +0x14 | In BloodMoney just forwarded, not in use
    };
}