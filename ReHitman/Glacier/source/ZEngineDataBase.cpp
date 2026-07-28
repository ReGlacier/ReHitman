#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ResourceCollection.h>
#include <Glacier/ZEngineGeomControl.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Com/Globals.h>
#include <Glacier/Com/CCom.h>

#include <Glacier/ScriptEngine/ScriptEngine.h>

#include <Glacier/Serializer/ZIOInputStream.h>
#include <Glacier/Filesystem/IOFilesystem_t.h>
#include <Glacier/Filesystem/ZSysFile.h>

#include <Glacier/EventBase/ZScheduledUpdate.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/EventBase/ZBaseConRout.h>

#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZCAMERA.h>

#include <Glacier/Render/ZRender.h>

#include <Glacier/ZSTL/ZPoolAllocRefTab.h>

#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>

#include <G1ConfigurationService.h>
#include <HF/HackingFramework.hpp>
#include <cassert>
#include <cstring>
#include <new>


namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE(float, g_fDisplayPercentTarget);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(float, g_fDisplayPercentTarget, 0x008BA060, 0.0f);

    namespace
    {
        int stricmpend(const char* str, const char* suffix)
        {
            size_t str_len = std::strlen(str);
            size_t suffix_len = std::strlen(suffix);

            if (str_len < suffix_len)
            {
                return 1;
            }

            const char* str_suffix_start = str + (str_len - suffix_len);
            return _stricmp(str_suffix_start, suffix); // or strcasecmp
        }
    }

    ZEngineDataBase::ZEngineDataBase(const char* pFileName)
    {
        m_rParticleControllerGeom = 0;
        m_bPackTime = false;

        ZRTString* pCurrentString = &m_ZMsgStrings[0];
        for (int i = 1023; i != -1; --i)
        {
            new (pCurrentString) ZRTString();
            ++pCurrentString;
        }

        Initialize(pFileName);
    }

    ZEngineDataBase::~ZEngineDataBase()
    {
        if (m_pScene)
        {
            ZUniMemory::Delete(m_pScene);
            m_pScene = nullptr;
        }

        if (m_pLocaleResources)
        {
            ZUniMemory::Delete(m_pLocaleResources);
            m_pLocaleResources = nullptr;
        }

        m_EventList.Clear();

        if (m_pPackedTreeData)
        {
            ZUniMemory::Free(m_pPackedTreeData);
            m_pPackedTreeData = nullptr;
        }
        m_lPackedTreeDataLength = 0;

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

        if (m_pScheduledUpdate)
        {
            ZUniMemory::Delete(m_pScheduledUpdate);
            m_pScheduledUpdate = nullptr;
        }

        if (m_pListUser)
        {
            ZUniMemory::Delete(m_pListUser);
            m_pListUser = nullptr;
        }

        if (m_pStaticBuffer)
        {
            ZUniMemory::Free(m_pStaticBuffer);

            m_pStaticBuffer = 0;
            m_lStaticBufferLength = 0;
        }

        if (m_pPackedAnims)
        {
            ZUniMemory::Free(m_pPackedAnims);
            m_pPackedAnims = nullptr;
        }

        FreeMsgValues();

        if (m_pRoomColiTreeData)
        {
            ZUniMemory::Free(m_pRoomColiTreeData);
            m_pRoomColiTreeData = nullptr;
        }

        if (m_pRoomInsideTreeData)
        {
            ZUniMemory::Free(m_pRoomInsideTreeData);
            m_pRoomInsideTreeData = nullptr;
        }
    }

    void ZEngineDataBase::PreLoad(ISerializerStream&)
    {
        ZEngineGeomControl::GetInstance().Clear();
    }

    void ZEngineDataBase::ExchangeObject(ISerializerStream& stream)
    {
        g_pSysInterface->LoadSave(stream);
        if (g_pSysInterface->m_pSoundDll)
        {
            // TODO: Finish me
        }

        // TODO: Finish me
    }

    void ZEngineDataBase::InitAllocSequencePercent(ZSWScene* pSceneWrapper, bool bPacked)
    {
        m_fDisplayPercent = 0.0f;
        g_fDisplayPercentTarget = 0.0f;
    }

    float ZEngineDataBase::SetAllocSequencePercent(ALLOCSEQUENCESTATUS Status, char const* pText, float fPercent)
    {
        constexpr float aPrecomputedValues[8] = {
            0.f,
            0.1f,
            0.2f,
            0.3f,
            0.4f,
            0.5f,
            0.8f,
            1.0f
        };

        if (Status == ALLOCSEQUENCESTATUS::AS_INCLUDESCENE)
        {
            return 0.0f;
        }

        const float fCurrentProgress = aPrecomputedValues[static_cast<int>(Status)];
        const float fFutureProgress = aPrecomputedValues[static_cast<int>(Status) + 1];

        g_fDisplayPercentTarget = fFutureProgress;

        const float fProgressValue = ((fFutureProgress - fCurrentProgress) * fPercent) + fCurrentProgress;

        if (m_fDisplayPercent + 0.002f < fProgressValue)
        {
            m_fDisplayPercent = fProgressValue;
            // TODO: Finish me after ZLoader_Sequence_Player will be reversed!
        }

        return m_fDisplayPercent;
    }

    void ZEngineDataBase::SoundUpdate()
    {
        if (!g_pSysInterface->m_pSoundDll)
            return;

        if (!IsPaused() && GetOnlyEventUpdate() == nullptr)
        {
            // TODO: Finish after ZSoundDllWintel reversed
        }

        // TODO: Finish after ZSoundDllWintel reversed
    }

    void ZEngineDataBase::MainLoop(bool bUpdateViews)
    {
        // TODO: Finish me after SysInput, Action:: and ZGameData will be reversed
    }

    
    const char* ZEngineDataBase::GetSceneName()
    {
        return m_FileName;
    }

    ZROOM* ZEngineDataBase::AllocRootGroup()
    {
        return reinterpret_cast<ZROOM*>(ZGeomBuffer::Instance().AllocGeom("ROOT", 0x100021, nullptr)->GetGeom());
    }
    
    void ZEngineDataBase::AllocSequence(struct ZSWScene* __formal)
    {
        // TODO: Finish me... later
    }

    bool ZEngineDataBase::ForceExtraGeom()
    {
        return true;
    }

    void ZEngineDataBase::CountNrGeoms(uint32_t& lNrBaseGeoms, uint32_t& lExtraGeomsSize, SGeomTypeCount& pGeomTypeCount, uint32_t lNrGeomTypes)
    {
        if (!m_pRoot)
        {
            ++lNrBaseGeoms;
            lExtraGeomsSize += 0x50;
        }

        // TODO: Finish me

        // end
        lExtraGeomsSize += 0x7800; // IOI?!
        lNrBaseGeoms += 0x112;
    }

    void ZEngineDataBase::LoadBoundTrees()
    {
        if (!m_lPackedTreeDataLength)
        {
            m_lPackedTreeDataLength = GetTreesSize();
            if (m_lPackedTreeDataLength == -1)
            {
                m_lPackedTreeDataLength = 0;
            }
        }

        if (m_lPackedTreeDataLength)
        {
            if (!m_pPackedTreeData)
            {
                m_pPackedTreeData = (uint8_t*)ZUniMemory::Allocate(m_lPackedTreeDataLength);
                GetTreesData(m_pPackedTreeData, m_lPackedTreeDataLength);
            }
        }
        else
        {
            m_lPackedTreeDataLength = 0;
        }
    }

    void ZEngineDataBase::CreateBoundTrees()
    {
        // TODO: Finish me
    }

    void ZEngineDataBase::CreateRoomTrees()
    {
        // Nothing
    }

    void ZEngineDataBase::LoadRoomTrees()
    {
        // TODO: Finish me after ZCollisionBase will be reversed
    }

    void ZEngineDataBase::CreateSoundGraph()
    {
        if (!g_pSysInterface->m_pSoundDll) return;

        // TODO: Finish me after ZSoundDllWintel reversed
    }

    void ZEngineDataBase::LoadSoundGraph()
    {
        if (!g_pSysInterface->m_pSoundDll) return;

        // TODO: Finish me after ZSoundDllWintel reversed
    }

    void ZEngineDataBase::RegisterZDefine(char const* pName, char*, int)
    {
        // Nothing
    }
    
    ZMSGID ZEngineDataBase::RegisterZMsg(char const* pMsgName, uint32_t lForcedValue, const char* pFile, int Line)
    {
        if (m_pZMessageHash)
        {
            m_pZMessageHash = ZUniMemory::New<ZPStrHash<unsigned int>>(1024);
        }

        MYSTR sMessage { pMsgName };
        sMessage.ToLower();
        const char* pFinalMessage = sMessage; 

        auto rMessage = ScriptEngine::GetRegisterZMessageID(pFinalMessage);;
        if (rMessage)
        {
            return rMessage;
        }

        auto* pFoundMessage = m_pZMessageHash->Get(pFinalMessage);
        if (pFoundMessage)
        {
            // Found in local hash table
            return static_cast<uint16_t>(*pFoundMessage);
        }

        // Calculate message id
        auto lMsgNumber = lForcedValue;
        if (!lMsgNumber)
        {
            ++m_iNumRegisteredMessages;
            lMsgNumber = m_iNumRegisteredMessages;
        }
        ZASSERT(m_iNumRegisteredMessages < MAXNRREGISTERMESSAGES);

        // Store entity to hashtable
        m_pZMessageHash->Put(pFinalMessage, lMsgNumber, true);

        // And to ZMSGID -> const char* table too
        m_ZMsgStrings[lMsgNumber] = pFinalMessage;

        return static_cast<ZMSGID>(lMsgNumber);
    }

    const char* ZEngineDataBase::GetZMsgName(ZMSGID lMsgNumber)
    {
        if (!lMsgNumber)
            return nullptr;

        if (lMsgNumber > MAXNRREGISTERMESSAGES)
        {
            return "";
        }

        return m_ZMsgStrings[lMsgNumber];
    }

    void ZEngineDataBase::CreateObjectFactories()
    {
        // Do nothing
    }

    bool ZEngineDataBase::StartUp()
    {
        // TODO: Finish me after ZRender interface & ZSoundDllWintel will be finished
        return false;
    }

    void ZEngineDataBase::CloseDown()
    {
        if (m_pPackedTreeData)
        {
            ZUniMemory::Free(m_pPackedTreeData);
            m_pPackedTreeData = nullptr;
        }

        m_lPackedTreeDataLength = 0;


        // TODO: Finish me after ZSoundDllWintel & g_pD3DDll & ZRender will be revered
    }
    
    void ZEngineDataBase::AddDlc(const char* pDllName)
    {
        g_pSysInterface->AddDll(pDllName);
    }

    void ZEngineDataBase::FreeDlcFiles()
    {
        // Do nothing
    }

    void ZEngineDataBase::GetDefaultDLCFiles(STRREFTAB* pDlcFiles)
    {
        // Do nothing
    }

    void ZEngineDataBase::ControlSceneChange()
    {
        if (!m_pScene->m_Changing)
        {
            return;
        }

        // Select next action
        ZScene::EToDo eToDo = m_pScene->m_SceneName[0] == '\0' ? ZScene::EToDo::TODO_Unload : ZScene::EToDo::TODO_UnloadAndLoad;

        if (!m_pScene->m_Loaded)
        {
            ZASSERT(m_pScene->m_SceneName[0] != '\0');
            eToDo = ZScene::EToDo::TODO_Load;
        }

        // Release render resources
        int iProcessedRenderers = 2;

        do
        {
            for (auto* pCurrentRender = g_pSysInterface->WindowFirst; pCurrentRender; pCurrentRender = pCurrentRender->Nxt)
            {
                pCurrentRender->ForceAllLeave();
            }

            --iProcessedRenderers;
        }
        while (iProcessedRenderers);

        // Finalize everything
        if (eToDo == ZScene::EToDo::TODO_Unload || eToDo == ZScene::EToDo::TODO_UnloadAndLoad)
        {
            PushValues(m_pScene);

            m_pScene->UnloadDoneNotify();
            g_pSysInterface->LockRefs();
            ZPoolAllocator::ReportHighWaterMarks();
            ZPoolAllocator::ResetAll();

            ZGEOM::SetPreferedStatus(ZGEOM::EStatus::STATUS_New);
            ZEventBase::SetPreferedStatus(ZEventBase::EStatus::STATUS_New);
        }

        if (eToDo == ZScene::EToDo::TODO_Unload)
        {
            g_pSysInterface->CloseDown();
        }
        else if (eToDo == ZScene::EToDo::TODO_Load || eToDo == ZScene::EToDo::TODO_UnloadAndLoad)
        {
            DoLoadScene();
        }
    }

    void ZEngineDataBase::UnloadScene()
    {
        m_lDontDrawFrame = 2;
        m_pScene->m_Changing = m_pScene->m_Loaded;
        m_pScene->m_SceneName[0] = '\0';
    }

    void ZEngineDataBase::LoadScene(char const* scene_name)
    {
        ZASSERT(strlen(scene_name) < sizeof(m_pScene->m_SceneName));

        m_pScene->m_Changing = true;

        char* dest = m_pScene->m_SceneName;
        const char* src = scene_name;

        while (*src != '\0')
        {
            *dest = (*src == '/') ? '\\' : *src;
            ++src;
            ++dest;
        }

        *dest = '\0';
    }

    void ZEngineDataBase::CheckAndMakeStaticContainer()
    {
        // Do nothing
    }

    void ZEngineDataBase::DoUnloadScene()
    {
        // TODO: Finish me
    }

    void ZEngineDataBase::FreeSceneMemory()
    {
        // TODO: Finish me
    }

    void ZEngineDataBase::PushValues(ZScene* pNewScene)
    {
        // TODO: Finish me
    }

    void ZEngineDataBase::InstallTextureBuffer()
    {
        // TODO: Finish me
    }

    uint32_t ZEngineDataBase::GetPrimsSize()
    {
        MYSTR sPrimPackFile = CalcCacheFileName(m_FileName, "prm");
        return g_pSysFile->GetSize(sPrimPackFile, false);
    }

    void ZEngineDataBase::GetPrimsData(void* pData, uint32_t lSize)
    {
        MYSTR sPrimPackFile = CalcCacheFileName(m_FileName, "prm");
        g_pSysFile->Load(sPrimPackFile, pData, lSize, 0, false);

        SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_TEXTURE, nullptr, 0.42f);
    }

    uint32_t ZEngineDataBase::GetGeomsSize()
    {
        MYSTR sGeomsFile = CalcCacheFileName(m_FileName, "gms");
        return g_pSysFile->GetSize(sGeomsFile, false);
    }

    void ZEngineDataBase::GetGeomsData(void* pData, uint32_t lSize)
    {
        MYSTR sGeomsFile = CalcCacheFileName(m_FileName, "gms");
        g_pSysFile->Load(sGeomsFile, pData, lSize, 0, false);

        SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_TEXTURE, nullptr, 0.86f);
    }

    IInputStream* ZEngineDataBase::CreatePropertyInputStream()
    {
        MYSTR sPropertiesName = CalcCacheFileName(m_FileName, "prp");

        const char* pszRealFileName = g_pSysFile->RemoveSysPath(sPropertiesName);
        auto* pZipFile = g_pSysFile->GetZipFile(pszRealFileName);
        auto* pFileHandle = pZipFile->open(sPropertiesName, IOFSAccess_t::IOFS_READ);

        return ZUniMemory::New<ZIOInputStream>(pZipFile, pFileHandle);
    }

    void ZEngineDataBase::CleanupPropertyData()
    {
        // Do nothing
    }

    uint32_t ZEngineDataBase::GetStaticSize()
    {
        MYSTR sBufFile = CalcCacheFileName(m_FileName, "buf");
        return g_pSysFile->GetSize(sBufFile, false);
    }

    void ZEngineDataBase::GetStaticData(void* pData, uint32_t lSize)
    {
        MYSTR sBufFile = CalcCacheFileName(m_FileName, "buf");
        g_pSysFile->Load(sBufFile, pData, lSize, 0, false);

        SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_TEXTURE, nullptr, 0.34f);
    }
    
    uint32_t ZEngineDataBase::GetTextureSize()
    {
        MYSTR sTexFile = CalcCacheFileName(m_FileName, "tex");
        return g_pSysFile->GetSize(sTexFile, false);
    }

    void ZEngineDataBase::GetTextureData(void* pData, uint32_t lSize)
    {
        SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_TEXTURE, nullptr, 0.0f);

        MYSTR sTexFile = CalcCacheFileName(m_FileName, "tex");
        g_pSysFile->Load(sTexFile, pData, lSize, 0, false);

        SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_TEXTURE, nullptr, 0.15f);
    }
    
    uint32_t ZEngineDataBase::GetMaterialsSize(void)
    {
        MYSTR sMatFile = CalcCacheFileName(m_FileName, "mat");
        return g_pSysFile->GetSize(sMatFile, false);
    }
    
    void ZEngineDataBase::GetMaterialsData(void* pData, uint32_t lSize)
    {
        MYSTR sMatFile = CalcCacheFileName(m_FileName, "mat");
        g_pSysFile->Load(sMatFile, pData, lSize, 0, false);

        SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_TEXTURE, nullptr, 0.25f);
    }

    uint32_t ZEngineDataBase::GetSoundDataSize()
    {
        MYSTR sSoundDataFile = CalcCacheFileName(m_FileName, "snd");
        return g_pSysFile->GetSize(sSoundDataFile, false);
    }
    
    void ZEngineDataBase::GetSoundData(void* pData, uint32_t lSize)
    {
        MYSTR sSndFile = CalcCacheFileName(m_FileName, "snd");
        g_pSysFile->Load(sSndFile, pData, lSize, 0, false);

        SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_TEXTURE, nullptr, 0.57f);
    }
    
    uint32_t ZEngineDataBase::GetWaveDataSize()
    {
        MYSTR sWaveFile = CalcCacheFileName(m_FileName, "wav");
        return g_pSysFile->GetSize(sWaveFile, false);
    }

    void ZEngineDataBase::GetWaveData(void* pData, uint32_t lSize)
    {
        MYSTR sWaveFile = CalcCacheFileName(m_FileName, "wav");
        g_pSysFile->Load(sWaveFile, pData, lSize, 0, false);

        SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_TEXTURE, nullptr, 0.0f);
    }

    uint32_t ZEngineDataBase::GetWaveHeaderDataSize()
    {
        MYSTR sWhdFile = CalcCacheFileName(m_FileName, "whd");
        return g_pSysFile->GetSize(sWhdFile, false);
    }

    void ZEngineDataBase::GetWaveHeaderData(void* pData, uint32_t lSize)
    {
        MYSTR sWhdFile = CalcCacheFileName(m_FileName, "whd");
        g_pSysFile->Load(sWhdFile, pData, lSize, 0, false);

        SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_TEXTURE, nullptr, 0.0f);
    }

    uint32_t ZEngineDataBase::GetAnimsSize()
    {
        MYSTR sAnmFile = CalcCacheFileName(m_FileName, "anm");
        return g_pSysFile->GetSize(sAnmFile, false);
    }

    void ZEngineDataBase::GetAnimsData(void* pData, uint32_t lSize)
    {
        MYSTR sAnmFile = CalcCacheFileName(m_FileName, "anm");
        g_pSysFile->Load(sAnmFile, pData, lSize, 0, false);

        SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_TEXTURE, nullptr, 0.70f);
    }

    uint32_t ZEngineDataBase::GetGeomFilesSize()
    {
        MYSTR sZgfFile = CalcCacheFileName(m_FileName, "zgf");
        return g_pSysFile->GetSize(sZgfFile, false);
    }

    void ZEngineDataBase::GetGeomFilesData(void* pData, uint32_t lSize)
    {
        MYSTR sZgfFile = CalcCacheFileName(m_FileName, "zgf");
        g_pSysFile->Load(sZgfFile, pData, lSize, 0, false);

        SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_TEXTURE, nullptr, 1.0f);
    }

    uint32_t ZEngineDataBase::GetRoomColiTreeSize()
    {
        MYSTR sRMCFile = CalcCacheFileName(m_FileName, "rmc");
        return g_pSysFile->GetSize(sRMCFile, false);
    }

    void ZEngineDataBase::GetRoomColiTreeData(void* pData, uint32_t lSize)
    {
        MYSTR sRMCFile = CalcCacheFileName(m_FileName, "rmc");
        g_pSysFile->Load(sRMCFile, pData, lSize, 0, false);
    }

    uint32_t ZEngineDataBase::GetRoomInsideTreeSize()
    {
        MYSTR sRMIFile = CalcCacheFileName(m_FileName, "rmi");
        return g_pSysFile->GetSize(sRMIFile, false);
    }

    void ZEngineDataBase::GetRoomInsideTreeData(void* pData, uint32_t lSize)
    {
        MYSTR sRMIFile = CalcCacheFileName(m_FileName, "rmi");
        g_pSysFile->Load(sRMIFile, pData, lSize, 0, false);
    }

    uint32_t ZEngineDataBase::GetGlobalColiTreeSize()
    {
        MYSTR sCOLFile = CalcCacheFileName(m_FileName, "col");
        return g_pSysFile->GetSize(sCOLFile, false);
    }

    void ZEngineDataBase::GetGlobalColiTreeData(void* pData, uint32_t lSize)
    {
        MYSTR sCOLFile = CalcCacheFileName(m_FileName, "col");
        g_pSysFile->Load(sCOLFile, pData, lSize, 0, false);
    }

    uint32_t ZEngineDataBase::GetGlobalStripColiTreeSize()
    {
        MYSTR sGSTFile = CalcCacheFileName(m_FileName, "gst");
        return g_pSysFile->GetSize(sGSTFile, false);
    }

    void ZEngineDataBase::GetGlobalStripColiTreeData(void* pData, uint32_t lSize)
    {
        MYSTR sGSTFile = CalcCacheFileName(m_FileName, "gst");
        g_pSysFile->Load(sGSTFile, pData, lSize, 0, false);
    }

    void ZEngineDataBase::EndAllocSequencePercent(ZSWScene*)
    {
        m_fDisplayPercent = 0.f;
    }

    CCom* ZEngineDataBase::GetSceneCom()
    {
        return &m_SceneCom;
    }

    std::intptr_t ZEngineDataBase::GetSceneVar(const char* varname) 
    {
        using CCom_t = int;

        auto ccom = (CCom_t*)GetSceneCom();
        if (!ccom) {
            return 0;
        }

        return HF::Hook::VFHook<CCom_t>::invoke<std::intptr_t, const char*, int>(ccom, 36, varname, 2);
    }

    ZSoundObject* ZEngineDataBase::SRefToPtr(Glacier::ZREF sref) 
    {
        // TODO: Reverse me after ZSoundDllWintel will be reversed
        assert(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_SRefToPtr != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_SRefToPtr != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((ZSoundObject*(__thiscall*)(ZEngineDataBase*, Glacier::ZREF))(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_SRefToPtr))(this, sref);
        }

        return 0;
    }

	ZGEOMCLASSINFO* ZEngineDataBase::GetGeomClassInfo(uint32_t typeId)
	{
        auto* pGeomClassInfo = ZGEOM::GetFactory().Find(typeId);
        if (!pGeomClassInfo)
        {
            printf("WARNING: Unable to get class info for geom type 0x%x\n", typeId);
        }

        return pGeomClassInfo;
	}

    ZROUTCLASSINFO* ZEngineDataBase::GetRoutClassInfo(const char* pszRoutInfo)
    {
        auto* pRoutClassInfo = ZBaseConRout::GetFactory().Find(pszRoutInfo);
        if (!pRoutClassInfo)
        {
            printf("WARNING: Unable to get rout class info for rout type '%s'\n", pszRoutInfo);
        }
        
    	return pRoutClassInfo;
    }

    void ZEngineDataBase::FreeMsgValues()
    {
        for (auto& entry : m_ZMsgStrings)
        {
            entry = {};
        }

        if (m_pZMessageHash)
        {
            ZUniMemory::Delete(m_pZMessageHash);
            m_pZMessageHash = nullptr;
        }

        m_iNumRegisteredMessages = 0;
        
        ZMessageResolver::ClearAll();
    }

    uint32_t ZEngineDataBase::GetTreesSize()
    {
        MYSTR sOctFile = CalcCacheFileName(m_FileName, "oct");
        return g_pSysFile->GetSize(sOctFile, false);
    }

    void ZEngineDataBase::GetTreesData(void* pData, unsigned int lSize)
    {
        MYSTR sOctFile = CalcCacheFileName(m_FileName, "oct");
        g_pSysFile->Load(sOctFile, pData, lSize, 0, false);
    }

    void ZEngineDataBase::DoLoadScene()
    {
        // TODO: Finish me
    }

    MYSTR ZEngineDataBase::CalcCacheFileName(MYSTR path, const char* new_ext)
    {
        const char* filename = strrchr(static_cast<const char*>(path), '\\');
        if (!filename)
        {
            filename = path;
        }

        // If no dot - add '.gms'
        if (path.Length() > 0 && !strchr(filename, '.'))
        {
            path += ".gms";
        }

        // Check current ext
        bool bIsGms = (stricmpend(path, ".gms") == 0);
        bool bIsZip = (stricmpend(path, ".zip") == 0);

        if (!bIsGms && !bIsZip)
        {
            // Unknown case
            return MYSTR("");
        }

        // For ZIP
        if (bIsZip)
        {
            char* last_slash = strrchr((char*)path, '\\');
            if (!last_slash)
            {
                return MYSTR("");
            }

            MYSTR filename_part(last_slash + 1);
            *last_slash = '\0';

            path += "\\";
            path += filename_part;
        }

        // Replace extension
        if (new_ext != nullptr)
        {
            char* ext_dot = strrchr((char*)path, '.');
            if (ext_dot)
            {
                memcpy(ext_dot + 1, new_ext, 3);
            }
        }

        return path;
    }

    CGlobalCom* ZEngineDataBase::GetGlobalCom() 
    {
        return (CGlobalCom*)g_pGlobalCom;
    }

    void ZEngineDataBase::Initialize(const char* pFileName)
    {
        m_pPathfinder4Data = nullptr;
        m_pEntityTracker = nullptr;
        m_AnimationManager = nullptr;
        m_pOnlyEventUpdate = nullptr;

        // Set inital seed
        auto lSeed = g_pSysInterface->TimeStampCounter(__FILE__, __LINE__);
        g_pSysInterface->SRand(lSeed, __FILE__, __LINE__);

        m_lDisableResources = 0;
        m_lDontDrawFrame = 3;
        m_pRoomColiTreeData = 0;
        m_pRoomInsideTreeData = 0;
        m_pListUser = 0;
        m_lNrEvents = 0;
        m_AnimIdCount = 1;
        m_bRunTime = 0;
        m_lLockMinMax = 0;
        m_FileName = pFileName;

        m_bPause = false;
        m_bFrozen = false;
        m_pGeomBuffer = nullptr;
        m_bInfAmmo = false;
        m_bLightDisplay = false;
        m_bCameraDisplay = false;
        m_bSoundDisplay = false;
        m_pPackedAnims = nullptr;
        m_lPackedAnimsLength = 0;
        m_pStaticBuffer = nullptr;
        m_lStaticBufferLength = 0;
        m_pPackedTreeData = nullptr;
        m_lPackedTreeDataLength = 0;
        m_lGlobalColiTreeLength = 0;
        m_pGlobalColiTreeData = nullptr;
        m_lGlobalStripColiTreeLength = 0;
        m_pGlobalStripColiTreeData = nullptr;
        m_pRoot = nullptr;
        m_iNumRegisteredMessages = 0;
        m_pZMessageHash = nullptr;
        m_lSceneDepth = 0;
        m_pScheduledUpdate = nullptr;

        m_pLocaleResources = ZUniMemory::New<ResourceCollection>();
        ZASSERT(g_pGlobalCom != nullptr);

        m_fEvenOutTimers = 0.0f;

        m_bDrawGizmoEnabled[0] = true;
        m_bDrawGizmoEnabled[1] = true;
        m_bDrawGizmoEnabled[2] = true;
        m_bDrawGizmoEnabled[3] = true;
        m_bDrawGizmoEnabled[4] = true;

        m_iDrawColiMask = 0;
        m_bStripViewEnabled = false;
        m_bWaterRenderingEnabled = true;

        m_pScene = ZUniMemory::New<ZScene>();

        m_SavingGame = false;
        m_LoadingGame = false;
        m_pLoadCallBack = nullptr;
    }

    void ZEngineDataBase::NewEventClass(ZEventBase* pEvent)
    {
        ++m_lNrEvents;
    }

    void ZEngineDataBase::DeleteEventClass(ZEventBase* pEvent)
    {
        --m_lNrEvents;
    }

    ZScheduledUpdate& ZEngineDataBase::GetEventScheduler()
    {
        if (!m_pScheduledUpdate)
        {
            m_pScheduledUpdate = ZUniMemory::New<ZScheduledUpdate>();
        }

        return *m_pScheduledUpdate;
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

    void ZEngineDataBase::DisableResources()
    {
        ++m_lDisableResources;
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

    void ZEngineDataBase::MarkRunTime()
    {
        if (!m_bRunTime)
        {
            m_bRunTime = true;
        }
    }

    void ZEngineDataBase::MarkNonRunTime()
    {
        if (m_bRunTime)
        {
            m_bRunTime = false;
        }
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

    ZCAMERA* ZEngineDataBase::CreateDefaultCam(ZCAMERA* pCamera)
    {
        ZVector3 vPosition { 0.f, 50.f, -200.f };
        ZMat3x3 mTransform {};
        mTransform.Reset();

        if (!pCamera)
        {
            pCamera = reinterpret_cast<ZCAMERA*>(m_pRoot->CreateGeom("DefaultCam", 0x400003, true));
        }

        pCamera->CameraCon |= 0x410000u;
        pCamera->BackCol = 0x00404040u;
        pCamera->SetMatPos(mTransform, vPosition);
        pCamera->AddEvent("ZCAMERA_PreviewCamera");
        pCamera->CameraListPri = 0x40000000;

        return pCamera;
    }

    void ZEngineDataBase::CorrectEditorDestGroup(SCompiledGeom* pCompiledGeom, ZGROUP* pCurrentDestGroup)
    {
        // Nothing
    }

    void ZEngineDataBase::PackHookMissingOnlyInitialize()
    {
        // Nothing
    }

    void ZEngineDataBase::CreatePackedStaticGameLevelData()
    {
        // Nothing
    }
    
    void ZEngineDataBase::LoadPackedStaticGameLevelData()
    {
        // TODO: Finish me
    }
}
