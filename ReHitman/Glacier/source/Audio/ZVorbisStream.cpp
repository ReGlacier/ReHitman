#include <Glacier/Audio/ZVorbisStream.h>
#include <Glacier/ZUniAssert.h>

#include <cstring>

namespace Glacier
{
    ZVorbisStream::~ZVorbisStream()
    {
    }

    bool ZVorbisStream::Init()
    {
        if (m_bInitialized)
            return true;

        m_iDecodeState = 0;
        ogg_sync_init(&m_OggSyncState);
        m_pOggBuffer = ogg_sync_buffer(&m_OggSyncState, 10240);

        int read = 0;
        if (m_pStream->GetData(m_pOggBuffer, 10240, read, 0) != 1)
            return false;

        ogg_sync_wrote(&m_OggSyncState, read);
        if (ogg_sync_pageout(&m_OggSyncState, &m_OggPage) != 1)
            return false;

        m_bInitialized = true;
        long streamOffset = m_OggSyncState.returned;

        ogg_stream_init(&m_OggStreamState, ogg_page_serialno(&m_OggPage));
        vorbis_info_init(&m_VorbisInfo);
        vorbis_comment_init(&m_VorbisComment);

        ZASSERT(ogg_stream_pagein(&m_OggStreamState, &m_OggPage) >= 0);
        ZASSERT(ogg_stream_packetout(&m_OggStreamState, &m_OggPacket) == 1);
        ZASSERT(vorbis_synthesis_headerin(&m_VorbisInfo, &m_VorbisComment, &m_OggPacket) >= 0);

        m_iHeaderCount = 0;
        while (m_iHeaderCount < 2)
        {
            if (ogg_sync_pageout(&m_OggSyncState, &m_OggPage) != 1)
                continue;

            streamOffset += m_OggSyncState.returned;
            ogg_stream_pagein(&m_OggStreamState, &m_OggPage);

            while (m_iHeaderCount < 2)
            {
                const int packetResult = ogg_stream_packetout(&m_OggStreamState, &m_OggPacket);
                if (packetResult == 0)
                    break;

                ZASSERT(packetResult > 0);
                vorbis_synthesis_headerin(&m_VorbisInfo, &m_VorbisComment, &m_OggPacket);
                ++m_iHeaderCount;
            }
        }

        ZASSERT(vorbis_synthesis_init(&m_VorbisDSPState, &m_VorbisInfo) >= 0);
        ZASSERT(vorbis_block_init(&m_VorbisDSPState, &m_VorbisBlock) >= 0);

        m_pStream->m_lFileLoopOffset = streamOffset;
        return true;
    }

    int ZVorbisStream::Run(int _size, int& _written)
    {
        _written = 0;
        int outputSample = 0;
        bool havePCM = m_iDecodeState == 3;

        while (true)
        {
            if (!havePCM)
            {
                if (m_iDecodeState != 1)
                {
                    bool packetReady = false;
                    while (!packetReady)
                    {
                        m_iResult = ogg_sync_pageout(&m_OggSyncState, &m_OggPage);
                        if (m_iResult == 0)
                        {
                            char* data = nullptr;
                            int read = 0;
                            const int streamResult = m_pStream->ReadFromStream(&data, 0x2000, read);
                            if (streamResult != 1)
                                return 0;

                            m_pOggBuffer = ogg_sync_buffer(&m_OggSyncState, read);
                            std::memcpy(m_pOggBuffer, data, read);
                            ogg_sync_wrote(&m_OggSyncState, read);
                            m_iResult = ogg_sync_pageout(&m_OggSyncState, &m_OggPage);
                            m_iDecodeState = 0;

                            if (ogg_page_eos(&m_OggPage))
                                return 0;
                        }

                        if (m_iResult < 0)
                            continue;

                        ogg_stream_pagein(&m_OggStreamState, &m_OggPage);
                        while (!packetReady)
                        {
                            m_iResult = ogg_stream_packetout(&m_OggStreamState, &m_OggPacket);
                            if (m_iResult == 0)
                                break;
                            if (m_iResult != 1)
                                continue;

                            if (vorbis_synthesis(&m_VorbisBlock, &m_OggPacket) == 0)
                                vorbis_synthesis_blockin(&m_VorbisDSPState, &m_VorbisBlock);
                            packetReady = true;
                        }
                    }
                }

                m_iPCMSamples = vorbis_synthesis_pcmout(&m_VorbisDSPState, &m_ppPCM);
                if (m_iPCMSamples <= 0)
                {
                    m_iDecodeState = 0;
                    continue;
                }

                m_iPCMCount = m_iPCMSamples < _size ? m_iPCMSamples : _size;
                m_iPCMOffset = 0;
                havePCM = true;
            }

            while (_written != _size)
            {
                int left = static_cast<int>(m_ppPCM[0][m_iPCMOffset] * 32768.0f);
                int right = static_cast<int>(m_ppPCM[1][m_iPCMOffset] * 32768.0f);

                if (left < -32768)
                    left = -32768;
                else if (left > 32767)
                    left = 32767;

                if (right < -32768)
                    right = -32768;
                else if (right > 32767)
                    right = 32767;

                m_pOutputBuffer[outputSample * 2] = static_cast<int16_t>(left);
                m_pOutputBuffer[outputSample * 2 + 1] = static_cast<int16_t>(right);
                _written += 4;
                ++outputSample;
                ++m_iPCMOffset;

                if (m_iPCMOffset >= m_iPCMCount)
                {
                    m_iDecodeState = 1;
                    vorbis_synthesis_read(&m_VorbisDSPState, m_iPCMCount);
                    havePCM = false;
                    break;
                }
            }

            if (_written == _size)
            {
                m_iDecodeState = 3;
                return 2;
            }
        }
    }

    void ZVorbisStream::End()
    {
        if (!m_bInitialized)
            return;

        ogg_stream_clear(&m_OggStreamState);
        vorbis_block_clear(&m_VorbisBlock);
        vorbis_dsp_clear(&m_VorbisDSPState);
        vorbis_comment_clear(&m_VorbisComment);
        vorbis_info_clear(&m_VorbisInfo);
        ogg_sync_clear(&m_OggSyncState);
        m_bInitialized = false;
    }
}
