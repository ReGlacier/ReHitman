#pragma once

#include <Glacier/Audio/ZIOStream.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>
#include <ogg/ogg.h>
#include <vorbis/codec.h>

namespace Glacier
{
#pragma pack(push, 4)
    class ZVorbisStream
    {
    public:
        virtual ~ZVorbisStream();
        virtual bool Init();
        virtual int Run(int _size, int& _written);
        virtual void End();

        void* m_pOwner;
        int32_t m_field08;
        ZIOStream* m_pStream;
        int32_t m_field10;
        int32_t m_iDecodeState;
        int32_t m_iHeaderCount;
        int32_t m_iResult;
        float** m_ppPCM;
        int32_t m_iPCMSamples;
        int32_t m_iPCMOffset;
        int32_t m_field2C;
        int32_t m_iPCMCount;
        void* m_pUnknown34;
        void* m_pUnknown38;
        char m_aStreamBuffer[0x2000];
        ogg_sync_state m_OggSyncState;
        ogg_stream_state m_OggStreamState;
        ogg_page m_OggPage;
        ogg_packet m_OggPacket;
        vorbis_info m_VorbisInfo;
        vorbis_comment m_VorbisComment;
        vorbis_dsp_state m_VorbisDSPState;
        vorbis_block m_VorbisBlock;
        char* m_pOggBuffer;
        bool m_bInitialized;
        char m_padding2305[7];
        int16_t* m_pOutputBuffer;
    };
#pragma pack(pop)

    RE_VERIFY_OFFSET(ZVorbisStream, m_OggSyncState, 0x203C);
    RE_VERIFY_OFFSET(ZVorbisStream, m_aStreamBuffer, 0x3C);
    RE_VERIFY_OFFSET(ZVorbisStream, m_OggStreamState, 0x2058);
    RE_VERIFY_OFFSET(ZVorbisStream, m_OggPage, 0x21C0);
    RE_VERIFY_OFFSET(ZVorbisStream, m_OggPacket, 0x21D0);
    RE_VERIFY_OFFSET(ZVorbisStream, m_VorbisInfo, 0x21F0);
    RE_VERIFY_OFFSET(ZVorbisStream, m_VorbisComment, 0x2210);
    RE_VERIFY_OFFSET(ZVorbisStream, m_VorbisDSPState, 0x2220);
    RE_VERIFY_OFFSET(ZVorbisStream, m_VorbisBlock, 0x2290);
    RE_VERIFY_OFFSET(ZVorbisStream, m_bInitialized, 0x2304);
    RE_VERIFY_OFFSET(ZVorbisStream, m_pOutputBuffer, 0x230C);
    RE_VERIFY_SIZE(ZVorbisStream, 0x2310);
}
