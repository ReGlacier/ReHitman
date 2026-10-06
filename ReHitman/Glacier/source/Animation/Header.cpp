#include <Glacier/Animation/Header.h>
#include <Glacier/Animation/ZBoneQuat.h>
#include <Glacier/Animation/Manager.h>
#include <Glacier/Animation/Model.h> // ZAngelBone
#include <Glacier/Animation/ZHumanState.h>
#include <Glacier/Animation/StreamPacker.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier::Animation
{
    Header* Header::RefToPtr(int rRef)
    {
        int lOffset = rRef - 1;
        if (!rRef)
            return nullptr;

        ZASSERT(lOffset >= 0 && lOffset < Animation::instance->m_Animcount);
        return &Animation::instance->m_Headers[lOffset];
    }

    int Header::GetRef()
    {
        if (!this)
            return 0;

        return Animation::instance->ToIndex(this) + 1;
    }

    int Header::Length()
    {
        return m_Frames - 1;
    }

    int32_t Header::GetSoundIndex() const
    {
        return m_SoundIndex;
    }

    float Header::GetBlendTime() const
    {
        return m_BlendFrames * TIME_SCALE;
    }

    bool Header::HasBone(Manager* manager, EBoneID boneId) const
    {
        if ((m_Mask & ZHM_HAS_BONES) != 0)
        {
            auto* pStates = &manager->m_Data[m_StateOffset];
            int32_t lStatesCount = *reinterpret_cast<uint32_t*>(pStates);
            auto* pState = reinterpret_cast<uint32_t*>(pStates) + 1;

            if (lStatesCount > 0)
            {
                int lIndex = 0;
                while (pState[lIndex] != boneId)
                {
                    if (++lIndex >= lStatesCount)
                        return false;
                }

                return true;
            }

            return false;
        }

        if (boneId == Ground)
            return m_GroundOffset != -1;

        if (boneId >= Ground || (m_Mask & ZHM_HAS_STATE) == 0)
        {
            if (m_QuatOffset != -1 && (m_Mask & ZHM_HAS_QUATS) != 0)
            {
                auto boneId2 = boneId;
                auto* pVectorIDs = reinterpret_cast<uint16_t*>(StreamPacker::GetVectorIds(&manager->m_Data[m_QuatOffset], boneId));
                if (boneId > Pelvis)
                {
                    int lVectorIdx = 0;
                    while (pVectorIDs[lVectorIdx] != boneId2)
                    {
                        if (++lVectorIdx >= boneId)
                        {
                            return false;
                        }
                    }

                    return true;
                }
            }

            return false;
        }

        return true;
    }

    void Header::DePackState(ZHumanState& state, float frame, Manager* manager, int bailAt)
    {
        if ((m_Mask & ZHM_HAS_STATE) == 0)
        {
            return;
        }

        if ((m_Mask & 0x4000000) != 0)
        {
            frame = floor(m_RealFPS * frame * TIME_SCALE);
        }

        if (frame >= 0.0f && static_cast<float>(m_Frames - 1) >= frame)
        {
            auto lIndex = Animation::instance->ToIndex(this);
            auto lBlock = StreamPacker::GetBlock(m_Frames, frame);

            if (manager->m_Cache)
            {
                auto* pEntry = manager->m_Cache->FindEntry(lIndex, lBlock);
                if (pEntry)
                {
                    StreamPacker::DePackBlockVector(&pEntry->m_Entry, m_Frames, frame, ZHumanState::m_StateId2Pos, reinterpret_cast<float*>(&state.m_Quats), bailAt);
                    return;
                }

                auto* pNewEntry = manager->m_Cache->AllocEntry(lIndex, lBlock);
                if (pNewEntry)
                {
                    StreamPacker::DePackBlock(&manager->m_Data[m_StateOffset], m_Frames, lBlock, &pNewEntry->m_Entry);
                    StreamPacker::DePackBlockVector(&pNewEntry->m_Entry, m_Frames, frame, ZHumanState::m_StateId2Pos, reinterpret_cast<float*>(&state.m_Quats), bailAt);
                    return;
                }
            }

            StreamPacker::DePackVector(&manager->m_Data[m_StateOffset], m_Frames, frame, ZHumanState::m_StateId2Pos, reinterpret_cast<float*>(&state.m_Quats), bailAt);
        }
        else
        {
            ZWARN2("Frame for animation is out of range (0<=%f<=%f) %s", frame, static_cast<float>(m_Frames - 1), m_Name);
        }
    }

    void Header::DePackQuats(int16_t* pId2Pos, ZBoneQuat* quat, float frame, Manager* manager)
    {
        if ((m_Mask & ZHM_HAS_QUATS) == 0)
        {
            return;
        }

        if ((m_Mask & 0x4000000) != 0)
        {
            frame = floor(m_RealFPS * frame * TIME_SCALE);
        }

        if (frame >= 0.0f)
        {
            const auto lLength = static_cast<float>(m_Frames - 1);
            if (frame <= lLength)
            {
                ZASSERT(frame >= 0 && frame <= m_Frames - 1);
                StreamPacker::DePackVector(&manager->m_Data[m_QuatOffset], m_Frames, frame, pId2Pos, &quat->m_vQuat[0], 0xFFFF);
            }
        }
    }

    void Header::DePackPose(int16_t* pPoseToPosLookup, float* poseBuf, float frame, Manager* manager)
    {
        if ((m_Mask & ZHM_HAS_POSES) == 0)
        {
            return;
        }

        ZASSERT(frame >= 0 && frame <= m_Frames - 1);
        if (m_PoseDataOffset != -1)
        {
            StreamPacker::DePackVector(&manager->m_Data[m_PoseDataOffset], m_Frames, frame, pPoseToPosLookup, poseBuf, 0xFFFF);
        }
    }

    void Header::DePackUncompressed(uint8_t* pBoneIdToIndexLookup, ZBoneQuat* pBones, float frame, Manager* manager)
    {
        ZASSERT(frame >= 0.0f && frame <= static_cast<float>(m_Frames - 1));

        const int32_t lBoneCount = *reinterpret_cast<const int32_t*>(&manager->m_Data[m_StateOffset]);
        const uint32_t* pBoneIds = reinterpret_cast<const uint32_t*>(&manager->m_Data[m_StateOffset]) + 1;
        const ZAngelBone* pQuats = reinterpret_cast<const ZAngelBone*>(&manager->m_Data[m_QuatOffset]);

        const int32_t lCurrentFrame = static_cast<int32_t>(std::floor(frame));
        int32_t lNextFrame = lCurrentFrame + 1;
        float t = frame - static_cast<float>(lCurrentFrame);

        if (lCurrentFrame == m_Frames - 1)
        {
            lNextFrame = lCurrentFrame;
            t = 0.0f;
        }

        for (int32_t i = 0; i < lBoneCount; ++i)
        {
            const uint32_t boneId = pBoneIds[i];
            const uint8_t lBoneIndex = pBoneIdToIndexLookup[boneId];

            if (boneId != Ground && lBoneIndex != 0xFF)
            {
                ZBoneQuat& bone = pBones[lBoneIndex];

                const ZAngelBone& kf0 = pQuats[i * m_Frames + lCurrentFrame];
                const ZAngelBone& kf1 = pQuats[i * m_Frames + lNextFrame];

                qpul(bone.m_vQuat, &kf0.m_Quat.i, &kf1.m_Quat.i, t);

                bone.m_vPosition[0] = kf0.m_Pos.x + (kf1.m_Pos.x - kf0.m_Pos.x) * t;
                bone.m_vPosition[1] = kf0.m_Pos.y + (kf1.m_Pos.y - kf0.m_Pos.y) * t;
                bone.m_vPosition[2] = kf0.m_Pos.z + (kf1.m_Pos.z - kf0.m_Pos.z) * t;
            }
        }
    }

    void Header::GetGround(Manager* manager, float frame, float* q, float* v, bool bMirror)
    {
        ZASSERT(frame >= 0 && frame <= m_Frames-1);
        if ((m_Mask & ZHM_UNCOMPRESSED) != 0)
        {
            // Uncompressed
            int32_t lStatesCount = *reinterpret_cast<int32_t*>(&manager->m_Data[m_StateOffset]);
            auto* pStates = reinterpret_cast<uint32_t*>(&manager->m_Data[m_StateOffset]) + 1;

            if (lStatesCount <= 0)
            {
                q[0] = 0.0f;
                q[1] = 0.0f;
                q[2] = 0.0f;
                q[3] = 1.0f;
                v[2] = 0.0f;
                v[1] = 0.0f;
                v[0] = 0.0f;
                return;
            }

            int32_t lStateIndex = 0;
            while (pStates[lStateIndex] != 56)
            {
                if (++lStateIndex >= lStatesCount)
                {
                    q[0] = 0.0f;
                    q[1] = 0.0f;
                    q[2] = 0.0f;
                    q[3] = 1.0f;
                    v[2] = 0.0f;
                    v[1] = 0.0f;
                    v[0] = 0.0f;
                    return;
                }
            }

            // State 56 found
            auto* pQuats = reinterpret_cast<ZAngelBone*>(&manager->m_Data[m_QuatOffset]);
            const int32_t lCurrentFrame = static_cast<int32_t>(std::floor(frame));
            int32_t lNextFrame = lCurrentFrame + 1;
            float t = static_cast<float>(frame - lCurrentFrame);

            if (lCurrentFrame == this->m_Frames - 1)
            {
                lNextFrame = lCurrentFrame;
                t = 0.0f;
            }

            int lStateOffset = lStateIndex * m_Frames;
            const ZAngelBone& kf0 = pQuats[lStateOffset + lCurrentFrame];
            const ZAngelBone& kf1 = pQuats[lStateOffset + lNextFrame];

            qpul(q, &kf0.m_Quat.i, &kf1.m_Quat.i, t);

            // lerp pos
            v[0] = kf0.m_Pos.x + (kf1.m_Pos.x - kf0.m_Pos.x) * t;
            v[1] = kf0.m_Pos.y + (kf1.m_Pos.y - kf0.m_Pos.y) * t;
            v[2] = kf0.m_Pos.z + (kf1.m_Pos.z - kf0.m_Pos.z) * t;

            if (bMirror)
            {
                mirrorquat(q);
                v[0] = -v[0];
            }
        }
        else
        {
            // Compressed
            if (m_GroundOffset == -1)
            {
                q[0] = 0.0f;
                q[1] = 0.0f;
                q[2] = 0.0f;
                q[3] = 1.0f;
                v[2] = 0.0f;
                v[1] = 0.0f;
                v[0] = 0.0f;
            }
            else
            {
                int16_t aBuffer[56];
                float aSamples[12];

                StreamPacker::DePackVector(&manager->m_Data[m_GroundOffset], m_Frames, frame, aBuffer, aSamples, 0xFFFF);
                q[0] = aSamples[0];
                q[1] = aSamples[1];
                q[2] = aSamples[2];
                q[3] = aSamples[3];
                v[0] = aSamples[9];
                v[1] = aSamples[10];
                v[2] = aSamples[11];

                if (bMirror)
                {
                    mirrorquat(q);
                    v[0] = -v[0];
                }
            }
        }
    }
}
