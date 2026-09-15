#include <Glacier/Audio/ZSoundBufferWintel.h>
#include <Glacier/Audio/ZSoundListener3D.h>
#include <Glacier/Audio/ZSynth.h>
#include <Glacier/Audio/ZSynthWintel.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <cmath>
#include <cstring>

namespace Glacier
{
    namespace
    {
        constexpr int ADPCMStepTable[] = {
            7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
            34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
            143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449,
            494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411,
            1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026,
            4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442,
            11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623,
            27086, 29794, 32767};

        bool ReadADPCMWord(SDecodeInfo& _info, uint32_t& _word)
        {
            if (_info.m_lBytesLeft < 4)
            {
                if (!_info.m_pStream)
                    return false;
                char* data = nullptr;
                int bytesRead = 0;
                _info.m_pStream->ReadFromStream(&data, 0x4000, bytesRead);
                if (!bytesRead)
                    return false;
                _info.m_pSource = reinterpret_cast<int32_t*>(data);
                _info.m_lBytesLeft = bytesRead;
            }
            _word = static_cast<uint32_t>(*_info.m_pSource++);
            _info.m_lBytesLeft -= 4;
            return true;
        }

        int CopyWaveDataADPCM(SDecodeInfo& _info, int16_t* _destination, const void* _source,
            bool _loop, int _wordsPerBlock, int _totalSamples, int _requestedSamples)
        {
            if (!_info.m_pSource && _source)
                _info.m_pSource = reinterpret_cast<int32_t*>(const_cast<void*>(_source));

            int remaining = _requestedSamples;
            while (remaining)
            {
                if (!_info.m_Loop)
                {
                    uint32_t header = 0;
                    if (!ReadADPCMWord(_info, header))
                        break;
                    _info.m_Previous = static_cast<int16_t>(header);
                    _info.m_Index = std::clamp(static_cast<int>((header >> 16) & 0xFF), 0, 88);
                    _info.m_Loop = 1;
                    _info.m_ByteSample = 0;
                    *_destination++ = static_cast<int16_t>(_info.m_Previous);
                    ++_info.m_dwSampleCnt;
                    --remaining;
                    if (!remaining)
                        break;
                }

                if (_info.m_dwSampleCnt >= _totalSamples)
                {
                    if (!_loop)
                    {
                        std::memset(_destination, 0, remaining * sizeof(int16_t));
                        break;
                    }
                    _info.m_Previous = 0;
                    _info.m_Index = 0;
                    _info.m_Loop = 0;
                    _info.m_ByteSample = 0;
                    _info.m_dwSampleCnt = 0;
                    _info.m_lBytesLeft = 0;
                    continue;
                }

                if (!_info.m_ByteSample && !ReadADPCMWord(_info, _info.m_dwSamples))
                    break;
                const int nibble = (_info.m_dwSamples >> (4 * _info.m_ByteSample)) & 0xF;
                const int step = ADPCMStepTable[_info.m_Index];
                int delta = step >> 3;
                if (nibble & 4) delta += step;
                if (nibble & 2) delta += step >> 1;
                if (nibble & 1) delta += step >> 2;
                if (nibble & 8) delta = -delta;

                _info.m_Previous = std::clamp(_info.m_Previous + delta, -32768, 32767);
                const int magnitude = nibble & 7;
                _info.m_Index = std::clamp(_info.m_Index + (magnitude >= 4 ? 2 * magnitude - 6 : -1), 0, 88);
                *_destination++ = static_cast<int16_t>(_info.m_Previous);
                ++_info.m_dwSampleCnt;
                --remaining;

                if (++_info.m_ByteSample == 8)
                {
                    _info.m_ByteSample = 0;
                    if (++_info.m_Loop >= _wordsPerBlock + 1)
                        _info.m_Loop = 0;
                }
            }
            return remaining;
        }

        float VectorAngle(const ZVector3& _left, const ZVector3& _right)
        {
            const float cosine = (_left.x * _right.x + _left.y * _right.y + _left.z * _right.z) /
                (_left.Length() * _right.Length());
            if (cosine > 1.0f)
                return 0.0f;
            if (cosine >= -1.0f)
                return std::acos(cosine);
            return 3.1415927f;
        }
    }

    ZSoundBufferWintel::ZSoundBufferWintel(ZSynth* _synth)
        : _ZSoundBuffer(_synth)
        , m_lLowpassHistoryLeft(0)
        , m_lLowpassHistoryRight(0)
        , m_lStreamRequestSize(0)
        , m_field63E4(0)
        , m_fVolumePercent(0.0f)
        , m_field63EC(0)
        , m_lCurrentStreamSegment(0)
    {
    }

    bool ZSoundBufferWintel::Create(const SWaveHeader* _wave, uint32_t _bufferType, uint32_t _flags)
    {
        m_lLowpassHistoryLeft = 0;
        m_lLowpassHistoryRight = 0;
        return _ZSoundBuffer::Create(_wave, _bufferType, _flags);
    }

    void ZSoundBufferWintel::Update(SStartSoundBase* _command)
    {
        _ZSoundBuffer::Update(_command);

        switch (_command->m_lType)
        {
        case 1:
            Update(reinterpret_cast<SStartSound2D*>(_command));
            break;
        case 2:
            Update(reinterpret_cast<SStartSound3D*>(_command));
            break;
        case 9:
            Update(reinterpret_cast<SStartSoundBFormat*>(_command));
            break;
        default:
            ZASSERT(false);
            break;
        }

        m_fVolumePercent = reinterpret_cast<SStartSound*>(_command)->m_fVolume;
        UpdateFilters();
        Update(reinterpret_cast<SStartSound*>(_command));
    }

    void ZSoundBufferWintel::Stop()
    {
        _ZSoundBuffer::Stop();
        m_VorbisStream.End();
    }

    int ZSoundBufferWintel::CopyWaveData()
    {
        int lockOffset = 0;
        uint32_t requestedSize = 0;
        if (m_eBufferType != SBT_DISCSTREAM)
            requestedSize = m_rWave->m_lDataSize;
        else if (!m_bReady)
            requestedSize = m_lBufferSize;
        else
        {
            if (!NeedData(&lockOffset))
                return 0;
            requestedSize = m_lBufferSize / 2;
        }
        if (!requestedSize)
            return 0;

        const char* residentSource = nullptr;
        uint32_t residentSize = 0;
        if (m_eBufferType == SBT_NORMAL)
        {
            residentSource = static_cast<ZSynthWintel*>(m_pSoundCon)->GetWaveform(m_rWave->m_lDataOffset);
            residentSize = m_rWave->m_lPackedSize;
        }

        if (m_rWave->m_iDataType == 4096)
        {
            m_VorbisStream.m_pStream = m_pStream;
            m_lStreamRequestSize = m_rWave->m_lDataSize;
            if (!m_VorbisStream.Init())
                return 0;
        }

        void* buffer1 = nullptr;
        void* buffer2 = nullptr;
        int size1 = 0;
        int size2 = 0;
        if (!LockBuffer(lockOffset, requestedSize, &buffer1, &size1, &buffer2, &size2))
            return 1;

        switch (m_rWave->m_iDataType)
        {
        case 17:
        {
            SDecodeInfo* decodeInfo = &m_Inf;
            ZBufferGroup* group = nullptr;
            if (m_dwBufferType)
            {
                group = m_pStream->m_pUserData;
                decodeInfo = &group->m_DecodeInfo;
                decodeInfo->m_pStream = m_pStream;
            }

            if (m_eBufferType == SBT_NORMAL)
            {
                decodeInfo->m_pSource = reinterpret_cast<int32_t*>(const_cast<char*>(residentSource));
                decodeInfo->m_lBytesLeft = m_rWave->m_lPackedSize;
            }

            if (m_dwBufferType == 2)
                std::memcpy(buffer1, group->m_Layers[0].m_pLayerData, requestedSize);
            else
            {
                const int wordsPerBlock = static_cast<int>(m_rWave->m_lNumChannels *
                    m_rWave->m_lBlockAlign / 4) - 1;
                int remaining = CopyWaveDataADPCM(*decodeInfo, static_cast<int16_t*>(buffer1),
                    residentSource, m_bLooping, wordsPerBlock, m_rWave->m_lNumSamples, size1 / 2);
                if (remaining && !buffer2)
                    std::memset(static_cast<int16_t*>(buffer1) + size1 / 2 - remaining,
                        0, remaining * sizeof(int16_t));
                else if (buffer2 && !remaining)
                    CopyWaveDataADPCM(*decodeInfo, static_cast<int16_t*>(buffer2), residentSource,
                        m_bLooping, wordsPerBlock, m_rWave->m_lNumSamples, size2 / 2);

                if (!m_bReady)
                    m_lPlayCursor = 0;
                if (m_dwBufferType == 1)
                    std::memcpy(group->m_Layers[0].m_pLayerData, buffer1, requestedSize);
            }
            break;
        }

        case 1:
            if (m_eBufferType == SBT_NORMAL)
            {
                std::memcpy(buffer1, residentSource, residentSize);
            }
            else if (!m_dwBufferType)
            {
                int firstRead = 0;
                const int firstStatus = m_pStream->GetData(static_cast<char*>(buffer1), size1, firstRead, 0);
                if (firstRead != size1 && firstStatus == 2)
                    std::memset(static_cast<char*>(buffer1) + firstRead, 0, size1 - firstRead);
                ApplyLowpass(static_cast<int16_t*>(buffer1), firstRead);

                int totalRead = firstRead;
                if (buffer2)
                {
                    int secondRead = 0;
                    const int secondStatus = m_pStream->GetData(static_cast<char*>(buffer2), size2, secondRead, 0);
                    if (secondRead != size2 && secondStatus == 3)
                        std::memset(static_cast<char*>(buffer2) + secondRead, 0, size2 - secondRead);
                    ApplyLowpass(static_cast<int16_t*>(buffer2), secondRead);
                    totalRead += secondRead;
                }
                m_lPlayCursor += totalRead - static_cast<int>(requestedSize);
            }
            else
            {
                auto* group = m_pStream->m_pUserData;
                if (m_dwBufferType == 1)
                {
                    uint32_t sampleCount = group->m_lSamples;
                    uint32_t sourceSize = requestedSize;
                    if (requestedSize != m_lBufferSize)
                        sampleCount /= 2;
                    if (m_rWave->m_iLayerInfo)
                        sourceSize = group->m_lSampleSize * sampleCount;
                    else
                        sampleCount = requestedSize;

                    char* source = static_cast<char*>(ZUniMemory::Allocate(sourceSize));
                    int bytesRead = 0;
                    m_pStream->GetData(source, sourceSize, bytesRead, 0);
                    if (bytesRead != static_cast<int>(sourceSize))
                        std::memset(source + bytesRead, 0, sourceSize - bytesRead);

                    if (!group->m_pFirstHeader)
                        std::memcpy(group->m_Layers[0].m_pLayerData, source, sampleCount);
                    else
                    {
                        const int layerCount = group->m_pFirstHeader->m_iLayerInfo & 0x7F;
                        for (int layerIndex = 0; layerIndex < layerCount; ++layerIndex)
                        {
                            auto& layer = group->m_Layers[layerIndex];
                            const int layerSize = layer.m_lBytesPerSample;
                            for (uint32_t sample = 0; sample < sampleCount; ++sample)
                                std::memcpy(layer.m_pLayerData + sample * layerSize,
                                    source + sample * group->m_lSampleSize + layer.m_lByteOffset, layerSize);
                        }
                    }
                    ZUniMemory::Free(source);
                }
                if (m_dwBufferType == 1 || m_dwBufferType == 2)
                {
                    const int8_t layerInfo = static_cast<int8_t>(m_rWave->m_iLayerInfo);
                    const int layerIndex = layerInfo < 0 ? 0 : layerInfo & 0x7F;
                    std::memcpy(buffer1, group->m_Layers[layerIndex].m_pLayerData, requestedSize);
                    ApplyLowpass(static_cast<int16_t*>(buffer1), requestedSize);
                }
            }
            break;

        case 4096:
        {
            m_VorbisStream.m_pOutputBuffer = static_cast<int16_t*>(buffer1);
            int written = 0;
            const int status = m_VorbisStream.Run(size1, written);
            if (status == 3)
            {
                UnlockBuffer(buffer1, size1, buffer2, size2);
                return 0;
            }
            if (written != size1)
            {
                const int remaining = size1 - written;
                if (m_bLooping)
                {
                    int secondWritten = 0;
                    m_VorbisStream.m_pOutputBuffer = reinterpret_cast<int16_t*>(
                        static_cast<char*>(buffer1) + written);
                    m_VorbisStream.Run(remaining, secondWritten);
                    written += secondWritten;
                }
                else
                    std::memset(static_cast<char*>(buffer1) + written, 0, remaining);
            }
            ApplyLowpass(static_cast<int16_t*>(buffer1), written);
            m_lPlayCursor += written - static_cast<int>(requestedSize);
            break;
        }

        default:
            break;
        }

        UnlockBuffer(buffer1, size1, buffer2, size2);
        return 1;
    }

    int ZSoundBufferWintel::GetNumStreamSamples()
    {
        return 0;
    }

    void ZSoundBufferWintel::Update(SStartSoundBFormat*)
    {
    }

    void ZSoundBufferWintel::UpdateFilters()
    {
    }

    void ZSoundBufferWintel::ApplyLowpass(int16_t* _data, int _size)
    {
        if (!_data || !_size || !m_bLowpassEnabled || m_rWave->m_lNumChannels != 2)
            return;

        const int frameCount = _size / 4;
        if (!frameCount)
            return;
        auto* original = static_cast<int16_t*>(ZUniMemory::Allocate(frameCount * 4));
        std::memcpy(original, _data, frameCount * 4);
        int previousLeft = m_lLowpassHistoryLeft;
        int previousRight = m_lLowpassHistoryRight;
        for (int frame = 0; frame < frameCount; ++frame)
        {
            const int sample = frame * 2;
            int left = ((1456 * _data[sample]) >> 16) + ((64079 * previousLeft) >> 16);
            int right = ((1456 * _data[sample + 1]) >> 16) + ((64079 * previousRight) >> 16);
            left = std::clamp(left, -32768, 32767);
            right = std::clamp(right, -32768, 32767);
            _data[sample] = static_cast<int16_t>(left);
            _data[sample + 1] = static_cast<int16_t>(right);
            previousLeft = _data[sample];
            previousRight = _data[sample + 1];
        }
        m_lLowpassHistoryLeft = _data[frameCount * 2 - 2];
        m_lLowpassHistoryRight = _data[frameCount * 2 - 1];

        const int originalWeight = (0x7FFF * m_lCrossFadePct) >> 7;
        const int filteredWeight = (0x7FFF * (127 - m_lCrossFadePct)) >> 7;
        for (int sample = 0; sample < frameCount * 2; ++sample)
            _data[sample] = static_cast<int16_t>(((originalWeight * original[sample]) >> 16) +
                ((filteredWeight * _data[sample]) >> 16));
        ZUniMemory::Free(original);
    }

    int ZSoundBufferWintel::Calc3DPan(SStartSound3D* _command)
    {
        auto* listener = static_cast<ZSoundListener3D*>(m_pSoundCon->m_pListener);
        const ZVector3 direction = *listener->GetPosition() - _command->m_vPosition;
        const float angle = VectorAngle(direction, *listener->GetEar());

        float pan = 0.0f;
        if (angle >= 1.5707999f && angle < 1.7454f)
            pan = (angle - 1.5707999f) * 11.454753f;
        else if (angle >= 1.7454f && angle < 2.0943999f)
            pan = (angle - 1.7454f) * 17.191978f + 2.0f;
        else if (angle >= 2.0943999f && angle <= 3.1415999f)
            pan = (angle - 2.0943999f) * 1.9098549f + 8.0f;
        else if (angle < 1.5707999f && angle > 1.3961999f)
            pan = (1.5707999f - angle) * -11.454753f;
        else if (angle <= 1.3961999f && angle > 1.0470999f)
            pan = -((1.3961999f - angle) * 17.187054f + 2.0f);
        else if (angle >= 0.0f && angle <= 1.0470999f)
            pan = -((1.0470999f - angle) * 1.9100374f + 8.0f);

        float distance = direction.Length();
        const float inverseMinDistance = 1.0f / _command->m_fMinDistance;
        const float distanceRatio = inverseMinDistance * distance;
        const int result = static_cast<int>((distanceRatio >= 1.0f ? pan : distanceRatio * pan) * 100.0f);

        if (distance == 0.0f)
            distance = 1.0f;
        if (distance > _command->m_fMinDistance)
            _command->m_fVolume /= inverseMinDistance * distance;
        return result;
    }

    int ZSoundBufferWintel::CalcBFormatPan(SStartSoundBFormat* _command)
    {
        _command->m_fVolume = 100.0f / std::pow(10.0f, _command->m_fW * 0.05f);
        return static_cast<int>((_command->m_fY * 0.70710677f - _command->m_fX * 0.70710677f -
            (_command->m_fX * 0.70710677f + _command->m_fY * 0.70710677f)) * 100.0f);
    }
}
