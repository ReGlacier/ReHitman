#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Serializer/IOutputSerializerStream.h>
#include <Glacier/Animation/ZRunMatPosAnim.h>
#include <Glacier/Animation/ZKeysDepack.h>
#include <Glacier/ZSTL/CHUNKFILE.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    ZRunMatPosAnim::ZRunMatPosAnim()
        : m_fStTime()
    {
        m_fFPS = 0.0f;
        m_bLoop = false;
        m_pChunkFile = nullptr;
        m_fFrameStart = 0.0f;
        m_fFrameEnd = 0.0f;
    }

    void ZRunMatPosAnim::SetStartTime(const TIMETYPE& sTimeType)
    {
        m_fStTime = sTimeType;
    }

    // PC 0x438560 / PS2 0x14BCF0. Clamps the frame start into the chunk's key range [data[1], data[2]].
    void ZRunMatPosAnim::SetFrameStart(int lFrameStart)
    {
        if (!m_pChunkFile)
        {
            ZASSERT(false); // PC/PS2: ERROR "Tried to used matpos chunk=0"
            return;
        }

        const int* pData = static_cast<const int*>(m_pChunkFile->Data());
        const int lMinKey = pData[1];
        const int lMaxKey = pData[2];
        m_fFrameStart = static_cast<float>(lFrameStart < lMinKey ? lMinKey : (lFrameStart > lMaxKey ? lMaxKey : lFrameStart));
    }

    // PC 0x4385C0 / PS2 0x14BDFC.
    void ZRunMatPosAnim::SetFrameEnd(int lFrameEnd)
    {
        if (!m_pChunkFile)
        {
            ZASSERT(false); // PC/PS2: ERROR "Tried to used matpos chunk=0"
            return;
        }

        const int* pData = static_cast<const int*>(m_pChunkFile->Data());
        const int lMinKey = pData[1];
        const int lMaxKey = pData[2];
        m_fFrameEnd = static_cast<float>(lFrameEnd < lMinKey || lFrameEnd > lMaxKey ? lMaxKey : lFrameEnd);
    }

    void ZRunMatPosAnim::SetChunkFile(CHUNKFILE* pChunkFile)
    {
        m_pChunkFile = pChunkFile;
    }

    // PC 0x438610 / PS2 0x14B4C0. Sets the chunk and the playback parameters.
    void ZRunMatPosAnim::Create(CHUNKFILE* pChunkFile, TIMETYPE startTime, bool bLooped, float fFPS)
    {
        m_pChunkFile = pChunkFile;
        m_fStTime = startTime;
        m_bLoop = bLooped;
        m_fFPS = fFPS;

        SetFrameStart(0);
        SetFrameEnd(-1);
    }

    // PC 0x4382C0 / PS2 0x14B650. Runs the mat/pos animation for the given time: computes the
    // current frame from the elapsed time and fps, depacks the pos (V3) + quat (Q4) keys and
    // writes the world matrix into pMatrix, the position into pPos. Returns the next/looping
    // frame or -1.0 when the anim has ended.
    float ZRunMatPosAnim::Run(TIMETYPE t, float* pMatrix, float* pPos, float fMaxKeyDist)
    {
        (void)fMaxKeyDist;

        if (!m_pChunkFile)
        {
            mreset(pMatrix);
            pPos[0] = 0.0f;
            pPos[1] = 0.0f;
            pPos[2] = 0.0f;
            return 0.0f;
        }

        const bool bReversed = m_fFrameEnd < m_fFrameStart;
        const int* pData = static_cast<const int*>(m_pChunkFile->Data());
        const int lNrFrames = pData[0];
        const uint8_t* pStaticBuffer = g_pEngineData->m_pStaticBuffer;
        const char* pQuatOffsets = reinterpret_cast<const char*>(pStaticBuffer + pData[4]);
        const char* pQuatTimes = reinterpret_cast<const char*>(pStaticBuffer + pData[5]);
        const int16_t* pQuatKeys = reinterpret_cast<const int16_t*>(pStaticBuffer + pData[6]);
        const char* pPosOffsets = reinterpret_cast<const char*>(pStaticBuffer + pData[7]);
        const char* pPosTimes = reinterpret_cast<const char*>(pStaticBuffer + pData[8]);
        const float* pPosKeys = reinterpret_cast<const float*>(pStaticBuffer + pData[9]);

        const float fElapsed = t - m_fStTime;
        float fFrame = fElapsed * m_fFPS + m_fFrameStart;
        if (bReversed)
            fFrame = -fElapsed * m_fFPS + m_fFrameStart;

        const int lFrameTicks = static_cast<int>(fFrame * 1024.0f);

        float fResult = 0.0f;
        bool bEnded = false;
        if ((m_bLoop || fFrame * 1024.0f > static_cast<float>(lFrameTicks) || bReversed)
            && (m_bLoop || static_cast<float>(lFrameTicks) > fFrame * 1024.0f || !bReversed))
        {
        }
        if (m_bLoop)
        {
            if (m_fFrameEnd != m_fFrameStart)
            {
                const int lRangeTicks = static_cast<int>(m_fFrameEnd - m_fFrameStart) << 10;
                int lRel = lFrameTicks - static_cast<int>(m_fFrameStart * 1024.0f);
                lRel %= lRangeTicks;
                fResult = static_cast<float>(lRel) * 0.0009765625f + m_fFrameStart;
            }
            else
            {
                fResult = m_fFrameStart;
            }
        }
        else
        {
            if (bReversed)
            {
                if (fFrame < m_fFrameStart)
                    fResult = m_fFrameStart;
                else if (fFrame > m_fFrameEnd)
                {
                    fResult = m_fFrameEnd;
                    bEnded = true;
                }
                else
                    fResult = fFrame;
            }
            else if (fFrame > m_fFrameEnd)
            {
                fResult = m_fFrameEnd;
                bEnded = true;
            }
            else if (fFrame < m_fFrameStart)
                fResult = m_fFrameStart;
            else
                fResult = fFrame;
        }

        // Normalized animation position within the key block.
        float fKeyPos = 0.0f;
        if (pData[3] != 0)
            fKeyPos = (fResult - static_cast<float>(pData[1])) / static_cast<float>(pData[3]);
        if (fKeyPos > static_cast<float>(lNrFrames - 1))
            fKeyPos = static_cast<float>(lNrFrames - 1);

        // Depack the position (V3) and the quaternion (Q4), compose the matrix.
        ZKeysDepack depack;
        float aPos[3];
        float aQuat[4];
        depack.DepackV3(aPos, fKeyPos, lNrFrames, pPosOffsets, pPosTimes, pPosKeys);
        depack.DepackQ4(aQuat, fKeyPos, lNrFrames, pQuatOffsets, pQuatTimes, pQuatKeys);
        quattomat(pMatrix, aQuat);
        pPos[0] = aPos[0];
        pPos[1] = aPos[1];
        pPos[2] = aPos[2];

        if (bEnded)
            return -1.0f;
        return m_bLoop ? m_fFrameStart : fResult;
    }

    float ZRunMatPosAnim::FPS() const
    {
        return m_fFPS;
    }

    float ZRunMatPosAnim::GetStart() const
    {
        return m_fFrameStart;
    }

    float ZRunMatPosAnim::GetEnd() const
    {
        return m_fFrameEnd;
    }

    // PS2 0x14B590. Returns the last key frame of the chunk (data[2]).
    float ZRunMatPosAnim::GetAnimEndTime() const
    {
        if (!m_pChunkFile)
        {
            ZASSERT(false); // PS2: printf("ERROR: ZRunMatPosAnim::GetAnimEndTime Tried to used matpos chunk=0")
            return 0.0f;
        }

        return static_cast<float>(reinterpret_cast<const int*>(m_pChunkFile->Data())[2]);
    }

    void ZRunMatPosAnim::LoadObject(IInputSerializerStream& stream)
    {
        // PC serializes this member directly as PT_Int32. Do not pass TIMETYPE through its
        // generic nested LoadSave path here: that would introduce a second "secs" field.
        stream.Exchange("m_fStTime", m_fStTime.secs);
        stream.Exchange("m_fFPS", m_fFPS);
        stream.Exchange("m_bLoop", m_bLoop);
        stream.Exchange("m_fFrameStart", m_fFrameStart);
        stream.Exchange("m_fFrameEnd", m_fFrameEnd);

        if (m_pChunkFile)
        {
            SetFrameStart(static_cast<int>(m_fFrameStart));
            SetFrameEnd(static_cast<int>(m_fFrameEnd));
        }
    }

    void ZRunMatPosAnim::SaveObject(IOutputSerializerStream& stream)
    {
        // Keep the PC field layout: m_fStTime is one raw fixed-point Int32 field.
        stream.Exchange("m_fStTime", m_fStTime.secs);
        stream.Exchange("m_fFPS", m_fFPS);
        stream.Exchange("m_bLoop", m_bLoop);
        stream.Exchange("m_fFrameStart", m_fFrameStart);
        stream.Exchange("m_fFrameEnd", m_fFrameEnd);
    }
}
