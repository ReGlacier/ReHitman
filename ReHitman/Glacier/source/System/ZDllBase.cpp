#include <Glacier/System/ZDllBase.h>
#include <Glacier/ZUniMemory.h>
#include <Windows.h>


namespace Glacier
{
    ZDllBase::~ZDllBase()
    {
        if (m_pName)
        {
            ZUniMemory::Free(m_pName);
        }
    }

    bool ZDllBase::CleanupBeforeCloseDown()
    {
        return true;
    }

    void ZDllBase::FreeMem()
    {}

    void ZDllBase::Init()
    {}

    void ZDllBase::End()
    {}

    void ZDllBase::PushScene(const char* pScene)
    {}

    void ZDllBase::PopScene()
    {}

    void ZDllBase::AllocSequenceStart()
    {}

    void ZDllBase::AllocSequenceEnd()
    {}

    void ZDllBase::InsertStart()
    {}

    void ZDllBase::InsertEnd()
    {}

    void ZDllBase::CrashFree()
    {}

    void ZDllBase::Set(void* pHandle, const char* psName)
    {
        if (m_pName)
        {
            ZUniMemory::Free(m_pName);
            m_pName = nullptr;
        }

        m_pName = (char*)ZUniMemory::Allocate(strlen(psName) + 1);
        strcpy(m_pName, psName);

        m_pHandle = pHandle;
    }

    void* ZDllBase::GetAddress(const char* pSymName)
    {
        if (!m_pHandle)
        {
            return nullptr;
        }

        return (void*)GetProcAddress((HMODULE)m_pHandle, (LPCSTR)pSymName);
    }

    void ZDllBase::PrintStatus()
    {}

    ZDllBase::ZDllBase() = default;
}
