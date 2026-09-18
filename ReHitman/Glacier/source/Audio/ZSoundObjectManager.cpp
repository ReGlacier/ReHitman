#include <Glacier/Audio/ZSoundObjectManager.h>
#include <Glacier/Audio/ZSoundObjectControllers.h>
#include <Glacier/ZSTL/ALLOCREF.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>
#include <new>

#ifdef GetObject
#undef GetObject
#endif

namespace Glacier
{
    ZSoundObjectManager::ZSoundObjectManager()
        : m_pAllocRef(nullptr)
        , m_pConvRefs(nullptr)
        , m_pObjects(nullptr)
        , m_lNumObjects(0)
        , m_lNextFree(0)
    {
    }

    ZSoundObjectManager::~ZSoundObjectManager()
    {
        Destroy();
    }

    void ZSoundObjectManager::Initialize()
    {
        m_lNextFree = 0;
        m_lNumObjects = 0;
        if (!m_pObjects)
            m_pObjects = static_cast<ZSoundObject*>(ZUniMemory::Allocate(512 * sizeof(ZSoundObject)));

        for (int i = 0; i < 511; ++i)
        {
            m_FreeMemTab[i] = i + 1;
            auto* object = new (&m_pObjects[i]) ZSoundObject();
            object->m_lKey = 0;
            object->m_pControllers = ZUniMemory::New<SoundObject::ZControllers>();
        }
        m_FreeMemTab[511] = -1;
        m_pObjects[511].m_lKey = 0;

        ZUniMemory::Delete(m_pAllocRef);
        ZUniMemory::Free(m_pConvRefs);
        m_pAllocRef = ZUniMemory::New<ALLOCREF>(9, 512);
        m_pConvRefs = static_cast<int32_t*>(ZUniMemory::Allocate(512 * sizeof(int32_t)));
        std::memset(m_pConvRefs, 0, 512 * sizeof(int32_t));
        m_pAllocRef->m_bCheckLock = false;
    }

    void ZSoundObjectManager::Destroy()
    {
        if (m_pObjects)
        {
            for (int i = 0; i < 511; ++i)
            {
                ZSoundObject& object = m_pObjects[i];
                ZUniMemory::Delete(object.m_pControllers);
                object.m_pControllers = nullptr;
                if (object.m_lKey)
                    object.~ZSoundObject();
            }
            ZUniMemory::Free(m_pObjects);
            m_pObjects = nullptr;
        }
        ZUniMemory::Delete(m_pAllocRef);
        m_pAllocRef = nullptr;
        ZUniMemory::Free(m_pConvRefs);
        m_pConvRefs = nullptr;
    }

    ZREF ZSoundObjectManager::AllocateObject()
    {
        if (m_lNumObjects >= 511)
            return 0;
        ZASSERT(m_lNextFree != -1);

        const int index = m_lNextFree;
        ZSoundObject* object = &m_pObjects[index];
        ZASSERT(object->m_lKey == 0);
        SoundObject::ZControllers* controllers = object->m_pControllers;
        new (object) ZSoundObject();
        object->m_pControllers = controllers;
        m_lNextFree = m_FreeMemTab[index];

        const ZREF reference = m_pAllocRef->NewRef();
        m_pConvRefs[reference & m_pAllocRef->m_lRefAnd] = index;
        object->m_lKey = reference;
        object->Initalize();
        object->m_lChainIdxCanPlayIn = 0;
        object->m_lChainIdxDontPlayIn = 0;
        object->m_lChainIdxNotifyTab = 0;
        object->m_lChainIdxNotifyStarted = 0;
        object->m_rGeomRef = 0;
        object->m_vLocalOrient = {0.0f, 0.0f, 1.0f};
        object->m_vOrientation = {0.0f, 0.0f, 1.0f};
        ++m_lNumObjects;
        ZASSERT(reference != 0);
        return reference;
    }

    void ZSoundObjectManager::FreeObject(ZREF _reference)
    {
        if (!_reference || !m_pAllocRef->FreeRef(_reference))
            return;
        const uint32_t slot = _reference & m_pAllocRef->m_lRefAnd;
        const int index = m_pConvRefs[slot];
        m_FreeMemTab[index] = m_lNextFree;
        m_pObjects[index].m_lKey = 0;
        m_lNextFree = index;
        m_pConvRefs[slot] = -1;
        --m_lNumObjects;
    }

    void ZSoundObjectManager::Reset()
    {
        m_lNextFree = 0;
        m_lNumObjects = 0;
    }

    void ZSoundObjectManager::PopScene()
    {
        Destroy();
        Reset();
    }

    void ZSoundObjectManager::Invalidate()
    {
        for (int i = 0; i < 512; ++i)
        {
            if (m_pObjects[i].m_lKey)
                m_pObjects[i].ClearBufferId();
        }
    }

    void ZSoundObjectManager::CorrectDelayValue(float _correction)
    {
        if (!m_pObjects)
            return;
        for (int i = 0; i < 512; ++i)
        {
            ZSoundObject& object = m_pObjects[i];
            if (object.m_lKey && object.m_fDelay != 0.0f && (object.m_lSoundFlags & 0x40))
                object.m_fDelay += _correction;
        }
    }

    ZSoundObject* ZSoundObjectManager::GetObject(uint32_t _reference)
    {
        return ConvRefToPtr(_reference);
    }

    ZSoundObject* ZSoundObjectManager::ConvRefToPtr(ZREF _reference)
    {
        if (!m_pAllocRef || !_reference || !m_pAllocRef->CheckRefActive(_reference))
            return nullptr;
        const int index = m_pConvRefs[_reference & m_pAllocRef->m_lRefAnd];
        return index == -1 ? nullptr : &m_pObjects[index];
    }

    int ZSoundObjectManager::GetNumAllocated() const
    {
        return m_lNumObjects;
    }
}
