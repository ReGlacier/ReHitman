#include <Glacier/Animation/ActiveAnimation.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/Animation/Manager.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZSTL/ZPoolAllocLinkSortRefTab.h>

#include <cstddef>
#include <cstring>
#include <new>


namespace Glacier::Animation
{
    namespace
    {
        struct AnimationCallback
        {
            uint32_t rGeomRef;
            uint32_t rCallReference;
            int16_t type;
            int16_t reserved;
            uint32_t callbackPadding;
            ActiveAnimation::CallBack_t callback;
            float sort;
        };

        static_assert(offsetof(AnimationCallback, callback) == 0x10);
        static_assert(offsetof(AnimationCallback, sort) == 0x18);

        AnimationCallback* AsCallback(uint32_t* pRef)
        {
            return reinterpret_cast<AnimationCallback*>(
                reinterpret_cast<char*>(pRef) - sizeof(uint32_t));
        }
    }

    ActiveAnimation::ActiveAnimation()
    {
        mode = 0x4000;
        Create(nullptr);
    }
    ActiveAnimation::~ActiveAnimation() = default;

    void ActiveAnimation::Create(ZLNKOBJ* pLnkObj)
    {
        m_fRemCallBackFrame = -1.0f;
        m_prtCallBacks = nullptr;
        m_pLnkObj = pLnkObj;
        sequenceId = 0;
    }

    void ActiveAnimation::Destroy(bool bUpdateAlways)
    {
        if (!m_prtCallBacks)
            return;

        if (bUpdateAlways)
        {
            RefRun run{};
            m_prtCallBacks->RunInitNxtRef(&run);

            while (auto* pRef = m_prtCallBacks->RunNxtRefPtr(&run))
            {
                auto* pCallback = reinterpret_cast<AnimationCallback*>(pRef);
                ZGEOM* pGeom = m_pLnkObj;

                if (pCallback->rCallReference)
                {
                    pGeom = ZGEOM::RefToPtr(pCallback->rCallReference);
                    if (!pGeom)
                        pGeom = m_pLnkObj;
                }

                if ((pCallback->type & 1) && pGeom && pCallback->callback)
                {
                    (pGeom->*pCallback->callback)(this, pCallback->sort, -1.0f, 0);
                }
            }
        }

        m_prtCallBacks->Clear();
        m_prtCallBacks->~ZPoolAllocLinkSortRefTab();
        s_AnimationCallbackAllocator.Free(m_prtCallBacks);
        m_prtCallBacks = nullptr;
    }
        
    void ActiveAnimation::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        stream.Exchange("Mode", mode);
        if ((mode & 7) == 0)
            return;

        int32_t lAnimId = 0;
        if (bSaving)
            lAnimId = Animation::instance->ToIndex(header);

        stream.Exchange("Id", lAnimId);

        if (!bSaving)
            header = Animation::instance->FromIndex(lAnimId);

        stream.Exchange("Frame", frame);
        stream.Exchange("StartFrame", startFrame);
        stream.Exchange("EndFrame", endFrame);
        stream.Exchange("Blend", blend);
        stream.Exchange("DFrame", deltaframe);
        stream.Exchange("SequenceId", sequenceId);

        ZREF rLnkObj = 0;
        if (bSaving && m_pLnkObj)
            rLnkObj = m_pLnkObj->GetRef();

        stream.Exchange("lnk ref", rLnkObj);

        if (!bSaving)
            m_pLnkObj = ZGEOM::RefCast<ZLNKOBJ>(rLnkObj);

        stream.Exchange("m_fRemCallBackFrame", m_fRemCallBackFrame);
    }
        
    void ActiveAnimation::UpdateCallBacks()
    {
        if (!m_prtCallBacks || m_fRemCallBackFrame == frame)
            return;

        m_fRemCallBackFrame = frame;

        RefRun run{};
        m_prtCallBacks->RunInitNxtRef(&run);

        while (auto* pRef = m_prtCallBacks->RunNxtRefPtr(&run))
        {
            auto* pCallback = reinterpret_cast<AnimationCallback*>(pRef);

            if (pCallback->sort > frame)
                return;

            if (pCallback->type & 2)
            {
                auto* pLoopedCallback = AsCallback(
                    m_prtCallBacks->AddSort(pCallback->rGeomRef, endFrame, 0));
                std::memcpy(pLoopedCallback, pCallback, sizeof(AnimationCallback) - sizeof(float));
                pLoopedCallback->sort = endFrame;
            }

            ZGEOM* pGeom = m_pLnkObj;
            if (pCallback->rCallReference)
            {
                pGeom = ZGEOM::RefToPtr(pCallback->rCallReference);
                if (!pGeom)
                    pGeom = m_pLnkObj;
            }

            if (pGeom && pCallback->callback)
            {
                const bool callbackResult =
                    (pGeom->*pCallback->callback)(this, frame, deltaframe, pCallback->rGeomRef);
                if (!callbackResult)
                    ZASSERT(false);
            }

            if (!m_prtCallBacks)
                return;
        }

        if (m_prtCallBacks->Count() == 0)
        {
            m_prtCallBacks->Clear();
            m_prtCallBacks->~ZPoolAllocLinkSortRefTab();
            s_AnimationCallbackAllocator.Free(m_prtCallBacks);
            m_prtCallBacks = nullptr;
        }
    }

    void ActiveAnimation::AddCallBack(
        float fFrameNr,
        CallBack_t pCallback,
        unsigned int rGeomRef,
        long int lType,
        unsigned int rCallReference)
    {
        if (!m_prtCallBacks)
        {
            void* pMemory = s_AnimationCallbackAllocator.Alloc(sizeof(ZPoolAllocLinkSortRefTab));
            ZASSERT(pMemory != nullptr);

            m_prtCallBacks = ::new (pMemory) ZPoolAllocLinkSortRefTab(
                &s_AnimationCallbackAllocator,
                32,
                6);
        }

        auto* pCallbackData = AsCallback(
            m_prtCallBacks->AddSort(rGeomRef, fFrameNr, 0));
        pCallbackData->rGeomRef = rGeomRef;
        pCallbackData->rCallReference = rCallReference;
        pCallbackData->type = static_cast<int16_t>(lType);
        pCallbackData->reserved = 0;
        pCallbackData->callbackPadding = 0;
        pCallbackData->callback = pCallback;
        pCallbackData->sort = fFrameNr;
    }
        
    void ActiveAnimation::AddCallBackAlways(float fFrameNr, CallBack_t pCallback, unsigned int rGeomRef)
    {
        AddCallBack(fFrameNr, pCallback, rGeomRef, 1, 0);
    }
        
    void ActiveAnimation::AddCallBackLoop(float fFrameNr, CallBack_t pCallback, unsigned int rGeomRef)
    {
        AddCallBack(fFrameNr, pCallback, rGeomRef, 2, 0);
    }

    void ActiveAnimation::AddMetaKeyCallBack(
        unsigned int lValue,
        CallBack_t pCallback,
        unsigned int rGeomRef,
        long int lType,
        unsigned int rCallReference)
    {
        if (!header || !Animation::instance)
            return;

        const uint32_t metaKeyCount = Animation::instance->GetMetaKeyDataLength(header->m_MetaDataOffset);
        auto* pMetaKeys = Animation::instance->GetMetaKeyData(header->m_MetaDataOffset);

        for (uint32_t i = 0; i < metaKeyCount; ++i)
        {
            if (pMetaKeys[i].lValue == lValue)
            {
                AddCallBack(
                    static_cast<float>(pMetaKeys[i].lFrame),
                    pCallback,
                    rGeomRef,
                    lType,
                    rCallReference);
            }
        }
    }
        
    void ActiveAnimation::AddMetaKeyCallBack(
        const char* pszValue,
        CallBack_t pCallback,
        unsigned int rGeomRef,
        long int lType)
    {
        if (!header || !Animation::instance || !pszValue)
            return;

        const uint32_t metaKeyCount = Animation::instance->GetMetaKeyDataLength(header->m_MetaDataOffset);
        auto* pMetaKeys = Animation::instance->GetMetaKeyData(header->m_MetaDataOffset);

        for (uint32_t i = 0; i < metaKeyCount; ++i)
        {
            const char* pValue = pMetaKeys[i].GetString();
            if (pValue && std::strcmp(pValue, pszValue) == 0)
            {
                AddCallBack(
                    static_cast<float>(pMetaKeys[i].lFrame),
                    pCallback,
                    rGeomRef,
                    lType,
                    0);
            }
        }
    }
        
    void ActiveAnimation::SetCurrentFrame(float fFrameNr)
    {
        frame = fFrameNr;
        m_fRemCallBackFrame = -1.0f;
    }

    
    char ActiveAnimation::s_AnimationCallbackBuffer[34816] { '\0' };
    ZPoolAllocator ActiveAnimation::s_AnimationCallbackAllocator {
        (char*)&ActiveAnimation::s_AnimationCallbackBuffer[0], 34816, "Animation::ActiveAnimation::s_AnimationCallbackAllocator", false
    };
}
