#include <Glacier/ZEngineDataBase.h>
#include <Glacier/EventBase/ZScheduledUpdate.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/ZEngineGeomControl.h>
#include <Glacier/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>

#include <G1ConfigurationService.h>
#include <HF/HackingFramework.hpp>
#include <cassert>


namespace Glacier
{
    void ZEngineDataBase::PreLoad(ISerializerStream&)
    {
        ZEngineGeomControl::GetInstance().Clear();
    }

    void ZEngineDataBase::EndAllocSequencePercent(ZSWScene*)
    {
        m_fDisplayPercent = 0.f;
    }

    CCom* ZEngineDataBase::GetSceneCom()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetSceneCom != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetSceneCom != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((CCom*(__thiscall*)(ZEngineDataBase*))(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetSceneCom))(this);
        }

        return 0;
    }

    std::intptr_t ZEngineDataBase::GetSceneVar(const char* varname) {
        using CCom_t = int;

        auto ccom = (CCom_t*)GetSceneCom();
        if (!ccom) {
            return 0;
        }

        return HF::Hook::VFHook<CCom_t>::invoke<std::intptr_t, const char*, int>(ccom, 36, varname, 2);
    }

    std::intptr_t ZEngineDataBase::SRefToPtr(Glacier::ZREF sref) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_SRefToPtr != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_SRefToPtr != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((std::intptr_t(__thiscall*)(ZEngineDataBase*, Glacier::ZREF))(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_SRefToPtr))(this, sref);
        }

        return 0;
    }

	ZOldTypeInfo* ZEngineDataBase::GetGeomClassInfo(uint32_t typeId)
	{
    	assert(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GeomGeomClassInfo != G1ConfigurationService::kNotConfiguredOption);
    	if (G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GeomGeomClassInfo != G1ConfigurationService::kNotConfiguredOption) {
		    return ((ZOldTypeInfo*(__thiscall*)(ZEngineDataBase*, uint32_t))(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GeomGeomClassInfo))(this, typeId);
    	}

    	return nullptr;
	}

    CCom* ZEngineDataBase::GetGlobalCom() {
        assert(G1ConfigurationService::G1API_InstanceAddress_CCom_g_globalCom != G1ConfigurationService::kNotConfiguredOption);

        if (G1ConfigurationService::G1API_InstanceAddress_CCom_g_globalCom != G1ConfigurationService::kNotConfiguredOption) {
            return reinterpret_cast<Glacier::CCom*>(G1ConfigurationService::G1API_InstanceAddress_CCom_g_globalCom);
        }

        return nullptr;
    }

    void ZEngineDataBase::NewEventClass(ZEventBase* pEvent)
    {
        ++m_lNrEvents;
    }

    ZScheduledUpdate* ZEngineDataBase::GetEventScheduler()
    {
        if (!m_pScheduledUpdate)
        {
            m_pScheduledUpdate = ZUniMemory::New<ZScheduledUpdate>();
        }

        return m_pScheduledUpdate;
    }

    void ZEngineDataBase::SetOnlyEventUpdate(ZEventBase* pEvent)
    {
        m_pOnlyEventUpdate = pEvent;
    }

    ZEventBase* ZEngineDataBase::GetOnlyEventUpdate() const
    {
        return m_pOnlyEventUpdate;
    }

    bool ZEngineDataBase::IsPaused() const
    {
        return m_bPause;
    }

    bool ZEngineDataBase::CheckInPackBuffer(void* ptr) const
    {
        return m_pStaticBuffer && ptr >= m_pStaticBuffer && ptr < &m_pStaticBuffer[m_lStaticBufferLength];
    }

    ZEventBase* ZEngineDataBase::AllocGeomCallEvent(ZGEOM* pGeom)
    {
        auto* pEvent = new ZEventBase();
        pEvent->m_fTimePassed = g_pSysInterface->m_fRealTime;
        pEvent->m_ClassCall = 1;

        return pEvent;
    }

    bool ZEngineDataBase::ResourcesDisabled() const
    {
        return m_lDisableResources != 0;
    }

    void ZEngineDataBase::EnableResources()
    {
        ZASSERT(ResourcesDisabled());
        --m_lDisableResources;
    }

    CListUser* ZEngineDataBase::GetListUser() const
    {
        return m_pListUser;
    }

    void ZEngineDataBase::DeleteCheck(void* ptr) const
    {
        if (CheckInPackBuffer(ptr))
            return;

        ZUniMemory::Free(ptr);
    }

    bool ZEngineDataBase::RunTime() const
    {
        return m_bRunTime;
    }

    void ZEngineDataBase::DeleteAllGeoms()
    {
        if (m_pRoot)
        {
            m_pRoot->Delete();
            m_pRoot = nullptr;
        }

        if (m_pGeomBuffer)
        {
            ZUniMemory::Delete(m_pGeomBuffer);
            m_pGeomBuffer = nullptr;
        }
    }

    void ZEngineDataBase::UnlockMinMax()
    {
        --m_lLockMinMax;
    }

    void ZEngineDataBase::LockMinMax()
    {
        ++m_lLockMinMax;
    }

    void ZEngineDataBase::UnlockScene()
    {
        // NOTE: Need check this code twice, but I guess it's ok

        m_lDontDrawFrame = 2;
        m_pScene->m_Changing = m_pScene->m_Loaded;
        m_pScene->m_SceneName[0] = '\0';
    }

    ZREF ZEngineDataBase::GetREFByName(const char* pszName) const
    {
        if (!pszName || !pszName[0]) return 0;

        auto* pFoundGeom = m_pRoot->FindGeom(pszName, nullptr);
        return pFoundGeom ? pFoundGeom->GetRef() : 0;
    }

    ZGEOM* ZEngineDataBase::GeomRefToPtr(ZREF rGeom) const
    {
        return m_pGeomBuffer->GeomRefToPtr(rGeom);
    }
}
