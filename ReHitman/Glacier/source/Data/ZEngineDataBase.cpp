#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZStaticGameLevelData.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/ResourceCollection.h>
#include <Glacier/Geom/ZEngineGeomControl.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/System/ZDllBase.h>
#include <Glacier/Com/Globals.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Debug/Globals.h>
#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Serializer/ZIOInputStream.h>
#include <Glacier/Serializer/ZTokenCache.h>
#include <Glacier/Filesystem/IOFilesystem_t.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/EventBase/ZBaseConRout.h>
#include <Glacier/EventBase/ZScheduledUpdate.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/EventBase/ZBaseConRout.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/Audio/ZSoundDllBase.h>
#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Action/ActionInterface.h>
#include <Glacier/ZSTL/ZPoolAllocRefTab.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/PF4/ZData.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Materials/BS_Runtime.h>
#include <Glacier/Debug/ZPushMemColor.h>
#include <Glacier/Debug/ZMemReadOut.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>


namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE(float, g_fDisplayPercentTarget);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(float, g_fDisplayPercentTarget, 0x008BA060, 0.0f);
    STATIC_GLOBAL_CLASS_INSTANCE(int, DEBUG_WhenToPrintMemory);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(int, DEBUG_WhenToPrintMemory, 0x008BA06C, 0);
    STATIC_GLOBAL_CLASS_INSTANCE(int, g_iLoadPropertiesProgress);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(int, g_iLoadPropertiesProgress, 0x008BA064, 0);
    STATIC_GLOBAL_CLASS_INSTANCE(int, g_iLoadPropertiesTotal);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(int, g_iLoadPropertiesTotal, 0x008BA068, 0);

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

        auto* pSoundDll = g_pSysInterface->GetSoundDll();

        if (!IsPaused() && GetOnlyEventUpdate() == nullptr)
        {
            pSoundDll->DispatchSoundEvents();
        }

        pSoundDll->RenderFrame();
    }

    void ZEngineDataBase::MainLoop(bool bUpdateViews)
    {
        static Action::ZHandle s_PauseAction { "Pause" };

        g_pSysInterface->UnlockRefs();
        {
            if (s_PauseAction.Digital())
            {
                m_bPause = !m_bPause;
            }

            if (g_pSysInterface->m_pSoundDll)
            {
                g_pSysInterface->GetSoundDll()->InitFrame();
            }

            if (!IsFrozen())
            {
                ZEngineGeomControl::GetInstance().UpdateMovedGeoms();
                m_pSaveObject = nullptr;

                FrameUpdate();
            }

            bool bDoDraw = m_lDontDrawFrame <= 0 ? bUpdateViews : false;
            if (!m_lDontDrawFrame)
            {
                m_lDontDrawFrame = -1;
                NetworkUpdate();
            }

            if (bDoDraw)
            {
                for (auto* pCurrentRender = g_pSysInterface->WindowFirst; pCurrentRender; pCurrentRender = pCurrentRender->Nxt)
                {
                    pCurrentRender->Update();
                }
            }

            SoundUpdate();
        }
        g_pSysInterface->LockRefs();

        if (m_pLoadCallBack)
        {
            m_pLoadCallBack->CallMe();
            m_pLoadCallBack = nullptr;
        }

        ControlSceneChange();
        if (g_pSysInterface->m_fAutoExitTime != 0.0f && static_cast<float>(g_pSysInterface->m_fActualTime) > static_cast<float>(g_pSysInterface->m_fAutoExitTime))
        {
            printf("\nAutoExit OK!\n");
            g_pSysInterface->m_fAutoExitTime = 0.0f;
            g_pSysInterface->m_bQuit = true;
        }

        if (g_pSysInterface->m_lFrameCount == DEBUG_WhenToPrintMemory)
        {
            if (ZMemReadOut::Exists())
            {
                ZMemReadOut::Instance().PrintStatus();
            }
        }
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

        if (lNrGeomTypes)
        {
            // TODO: Finish me
        }

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
        if (!m_pPackedTreeData)
            return;

        PUSH_MEMORY_COLOR(0x008080FCu);

        if (*reinterpret_cast<uint32_t*>(m_pPackedTreeData))
        {
            m_pRoot->MakeStaticContainer(true);
        }

        char* pBufferStart = reinterpret_cast<char*>(m_pPackedTreeData + 0x10);

        for (auto* pBaseGeom = m_pRoot->BaseGeom(); pBaseGeom; )
        {
            auto* pGeom = pBaseGeom->GetGeom();
            const bool isGroup = pGeom
                ? (pGeom->GetObjectId() & ZGROUP::m_Mask) == ZGROUP::m_Id
                : pBaseGeom->IsDerivedFrom<ZGROUP>();

            if (isGroup)
            {
                auto* pTreeGroup = pGeom
                    ? geom_cast<ZTreeGroup>(pGeom)
                    : static_cast<ZTreeGroup*>(nullptr);
                if (pTreeGroup && pTreeGroup->IsStaticContainer())
                {
                    pBufferStart = pTreeGroup->LoadBoundTrees(pBufferStart);
                }
            }

            m_pRoot->RecurGetNext(&pBaseGeom);
        }

        for (; *reinterpret_cast<int32_t*>(pBufferStart) != -1; pBufferStart = ZCollisionBase::GetCollisionInterface()->LoadInternColiTree(pBufferStart))
        {
        }

        const auto* end = m_pPackedTreeData + m_lPackedTreeDataLength;
        const auto* uniqueInfo = reinterpret_cast<const uint8_t*>(pBufferStart + 4);
        const uint32_t size = uniqueInfo <= end
            ? static_cast<uint32_t>(end - uniqueInfo)
            : 0;
        ZCollisionBase::GetCollisionInterface()->LoadUniqueSubStripInfo(
            reinterpret_cast<SUniqueSubStripInfo*>(pBufferStart + 4), size);
    }

    void ZEngineDataBase::CreateRoomTrees()
    {
        // Do nothing
    }

    void ZEngineDataBase::LoadRoomTrees()
    {
        const uint32_t roomColiTreeSize = GetRoomColiTreeSize();
        if (roomColiTreeSize != static_cast<uint32_t>(-1))
        {
            PUSH_MEMORY_COLOR(0x008080FCu);

            m_pRoomColiTreeData = static_cast<uint8_t*>(ZUniMemory::Allocate(roomColiTreeSize));
            GetRoomColiTreeData(m_pRoomColiTreeData, roomColiTreeSize);
            ZCollisionBase::GetCollisionInterface()->InstallCollisionBuffer(
                reinterpret_cast<char*>(m_pRoomColiTreeData), roomColiTreeSize);
        }

        const uint32_t roomInsideTreeSize = GetRoomInsideTreeSize();
        if (roomInsideTreeSize != static_cast<uint32_t>(-1))
        {
            PUSH_MEMORY_COLOR(0x008080FCu);

            m_pRoomInsideTreeData = static_cast<uint8_t*>(ZUniMemory::Allocate(roomInsideTreeSize));
            GetRoomInsideTreeData(m_pRoomInsideTreeData, roomInsideTreeSize);
            ZCollisionBase::GetCollisionInterface()->InstallInsideBuffer(
                reinterpret_cast<char*>(m_pRoomInsideTreeData), roomInsideTreeSize);
        }
    }

    void ZEngineDataBase::CreateSoundGraph()
    {
        if (!g_pSysInterface->m_pSoundDll) return;

        // TODO: Finish me after ZSoundDllWintel reversed
    }

    void ZEngineDataBase::LoadSoundGraph()
    {
        if (!g_pSysInterface->m_pSoundDll) return;

        PUSH_MEMORY_COLOR(0x7777u);

        uint32_t lSize = GetSoundGraphSize();
        if (lSize > 0)
        {
            void* pData = ZUniMemory::Allocate(lSize);
            GetSoundGraphData(pData, lSize);

            // TODO: Finish me after ZSoundDllWintel reversed
        }
    }

    void ZEngineDataBase::RegisterZDefine(char const* pName, char*, int)
    {
        // Nothing
    }

    ZMSGID ZEngineDataBase::RegisterZMsg(char const* pMsgName, uint32_t lForcedValue, const char* pFile, int Line)
    {
        if (!m_pZMessageHash)
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

        if (g_pSysInterface->m_pSoundDll)
            g_pSysInterface->m_pSoundDll->PopScene();

        FreeDlcFiles();

        if (g_pRenderDll)
            g_pRenderDll->PopScene();
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
        if (m_pEntityTracker)
        {
            ZUniMemory::Delete(m_pEntityTracker);
            m_pEntityTracker = nullptr;
        }

        // TODO: Finish me
        if (m_AnimationManager)
        {
            ZUniMemory::Delete(m_AnimationManager);
        }

        m_AnimationManager = nullptr;
        DeleteBoundTrees();

        // TODO: Finish me

        FreeRoutsLists();
        FreeScheduledUpdate();
        if (auto* pSGD = ZStaticGameLevelData::Instance())
        {
            pSGD->Destroy();
        }
        ZEngineGeomControl::GetInstance().Clear();
        FreeLightTable();
        m_SceneCom.Clear();
        if (m_pStaticBuffer)
        {
            ZUniMemory::Free(m_pStaticBuffer);
            m_pStaticBuffer = nullptr;
            m_lStaticBufferLength = 0;
        }
        if (m_pPackedAnims)
        {
            ZUniMemory::Free(m_pPackedAnims);
            m_pPackedAnims = nullptr;
            m_lPackedAnimsLength = 0;
        }
        g_pSysInterface->FreeActionMap();
        FreeMsgValues();
        UnlockMinMax();
        // TODO: Finish me after g_pGameDataFactory reversed
    }

    void ZEngineDataBase::PushValues(ZScene* pNewScene)
    {
        --m_lSceneDepth;
        g_pSysInterface->UnlockRefs();

        if (m_pPackedTreeData)
        {
            ZUniMemory::Free(m_pPackedTreeData);
            m_pPackedTreeData = nullptr;
        }
        m_lPackedTreeDataLength = 0;

        CloseDown();

        g_pSysInterface->LockRefs();
        g_pSysFile->RemoveAllBigs();

        m_pPackedAnims = pNewScene->_pPackedAnims;
        m_lPackedAnimsLength = pNewScene->_lPackedAnimsLength;
        m_pStaticBuffer = pNewScene->_pStaticBuffer;
        m_lStaticBufferLength = pNewScene->_lStaticBufferLength;
        m_pRoot = pNewScene->_pRoot;
        m_lLockMinMax = pNewScene->_lLockMinMax;
        m_pPackedTreeData = pNewScene->_pPackedTreeData;
        m_pGeomBuffer = pNewScene->_pGeomBuffer;

        if (g_pSysInterface->WindowFirst)
            g_pSysInterface->WindowFirst->RemoveCameras();

        if (pNewScene->_pBigFiles)
            g_pSysFile->m_pBigFiles = pNewScene->_pBigFiles;

        std::memcpy(&g_pSysInterface->FrameTime.secs, &pNewScene->_FrameTime, sizeof(int32_t));
        std::memcpy(&g_pSysInterface->PreFrameTime.secs, &pNewScene->_PreFrameTime, sizeof(int32_t));
        g_pSysInterface->ResetTime();
        std::memcpy(&g_pSysInterface->m_fActualTime.secs, &pNewScene->_ActTime, sizeof(int32_t));
    }

    void ZEngineDataBase::InstallTextureBuffer()
    {
        PUSH_MEMORY_COLOR(0xE0E0u);

        uint32_t lTextureSize = GetTextureSize();
        auto* pTextureData = ZUniMemory::Allocate(lTextureSize);

        GetTextureData(pTextureData, lTextureSize);

        g_pRenderDll->InstallTextureBuffer(pTextureData, lTextureSize);
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

    ZREF ZEngineDataBase::GetSceneVar(const char* varname) const
    {
        // TODO: Finish me
        return 0;
    }

    ZSoundObject* ZEngineDataBase::SRefToPtr(Glacier::ZREF sref)
    {
        if (!g_pSysInterface->m_pSoundDll)
            return nullptr;

        return g_pSysInterface->GetSoundDll()->SRefToPtr(sref);
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

    uint32_t ZEngineDataBase::GetStaticGameLevelDataSize()
    {
        MYSTR sStaticGameLevelFile = CalcCacheFileName(m_FileName, "sgd");
        return g_pSysFile->GetSize(sStaticGameLevelFile, false);
    }

    void ZEngineDataBase::GetStaticGameLevelData(void* pData, unsigned int lSize)
    {
        MYSTR sStaticGameLevelFile = CalcCacheFileName(m_FileName, "sgd");
        g_pSysFile->Load(sStaticGameLevelFile, pData, lSize, 0, false);
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

    bool ZEngineDataBase::CheckInPackBuffer(const void* ptr) const
    {
        return m_pStaticBuffer && ptr >= m_pStaticBuffer && ptr < &m_pStaticBuffer[m_lStaticBufferLength];
    }

    bool ZEngineDataBase::CheckInAnimBuffer(const void* ptr) const
    {
        return m_pPackedAnims && ptr >= m_pPackedAnims && ptr < reinterpret_cast<const uint8_t*>(&m_pPackedAnims) + m_lPackedAnimsLength;
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

    bool ZEngineDataBase::MinMaxLocked() const
    {
        return m_lLockMinMax > 0;
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
        PUSH_MEMORY_COLOR(0xFFFF40u);
        ZStaticGameLevelData::Create();

        const uint32_t dataSize = GetStaticGameLevelDataSize();
        if (dataSize == static_cast<uint32_t>(-1))
            return;

        void* data = ZUniMemory::Allocate(dataSize);
        GetStaticGameLevelData(data, dataSize);

        if (auto* staticData = ZStaticGameLevelData::Instance())
            staticData->Load(data);
    }

    void ZEngineDataBase::NetworkUpdate()
    {
        // TODO: Finish me (sub_69D620)
    }

    uint32_t ZEngineDataBase::GetNextAnimId()
    {
        ++m_AnimIdCount;
        if (m_AnimIdCount == -1)
        {
            ++m_AnimIdCount;
        }

        return m_AnimIdCount;
    }

    void ZEngineDataBase::SetLoadCallBack(ILoadCallBack* pCallBack)
    {
        m_pLoadCallBack = pCallBack;
    }

    bool ZEngineDataBase::IsLoadingGame() const
    {
        return m_LoadingGame;
    }

    bool ZEngineDataBase::IsSavingGame() const
    {
        return m_SavingGame;
    }

    void ZEngineDataBase::ScheduledUpdate()
    {
        if (m_pScheduledUpdate)
        {
            m_pScheduledUpdate->ScheduleEvents();
        }
    }

    void ZEngineDataBase::FrameUpdate()
    {
        PUSH_MEMORY_COLOR(0xFFFFFFu);

        if (g_pGameData)
        {
            g_pGameData->PreFrameUpdate();
        }

        // TODO: Uncomment me after ZDrawDebugTimer reversed
        // if (g_pDrawDebugTimer)
        // {
        //     g_pDrawDebugTimer->StartFastTimer();
        // }

        m_EventList.FrameUpdate();

        // TODO: Uncomment me after ZDrawDebugTimer reversed
        // if (g_pDrawDebugTimer)
        // {
        //     g_pDrawDebugTimer->EndFastTimer();
        // }

        if (g_pGameData)
        {
            g_pGameData->PostFrameUpdate();
        }
    }

    bool ZEngineDataBase::IsFrozen() const
    {
        return m_bFrozen;
    }

    void ZEngineDataBase::PauseScene(bool bPause)
    {
        if (m_bPause != bPause)
        {
            // DronCode: I'm not sure about this part. In PS2 used time save/update in this case, but in PC nothing
            m_bPause = bPause;
        }
    }

    void ZEngineDataBase::FreezeScene(bool bFreeze)
    {
        PauseScene(bFreeze);
        m_bFrozen = bFreeze;
    }

    ZREF ZEngineDataBase::SPtrToRef(ZSoundObject* pSoundObject) const
    {
        if (!pSoundObject)
            return 0;

        if (auto* pSoundDll = g_pSysInterface->GetSoundDll())
            return pSoundDll->SPtrToRef(pSoundObject);

        return 0;
    }

    void ZEngineDataBase::PurgePrimBuffer()
    {
        // TODO: Finish me (PC 0045CB20)
    }

    void ZEngineDataBase::InitPathfinder4Data(const char* pBuffer)
    {
        const auto lDataSize = *reinterpret_cast<const uint32_t*>(pBuffer);
        if (*pBuffer != 0xFFFFFFFCu && lDataSize > 0)
        {
            auto* pPathFinderMemBlock = ZUniMemory::Allocate(lDataSize);
            memcpy(pPathFinderMemBlock, pBuffer + 4, lDataSize);
            m_pPathfinder4Data = PF4::CreatePathFinder(pPathFinderMemBlock);
            m_pEntityTracker = ZUniMemory::New<ZEntityTracker>(m_pPathfinder4Data);
        }
    }

    bool ZEngineDataBase::InitPhysicsData(const char* pBuffer)
    {
        // NOTE: In PC this method to nothing, but in beta PS2 this method do
        // return ZDynamicsExtendRuntime::ReadDataBuffer(pBuffer);
        //
        // DronCode: PC will ignore all things here, I'm not sure about this code.
        return true;
    }

    bool ZEngineDataBase::IsDrawGizmoEnabled(EGizmoType eType) const
    {
        return m_bDrawGizmoEnabled[eType];
    }

    void ZEngineDataBase::EnableDrawGizmo(EGizmoType eType, bool bEnabled)
    {
        m_bDrawGizmoEnabled[eType] = bEnabled;
    }

    void ZEngineDataBase::FreeRoutsLists()
    {
        m_EventList.Clear();
    }

    void ZEngineDataBase::FreeScheduledUpdate()
    {
        if (!m_pScheduledUpdate)
            return;

        ZUniMemory::Delete(m_pScheduledUpdate);
        m_pScheduledUpdate = nullptr;
    }

    void ZEngineDataBase::FreeLightTable()
    {
        if (m_pListUser)
        {
            ZUniMemory::Free(m_pListUser);
            m_pListUser = nullptr;
        }
    }

    void ZEngineDataBase::DeleteBoundTrees()
    {
        if (m_pPackedTreeData)
        {
            ZUniMemory::Free(m_pPackedTreeData);
        }

        m_pPackedTreeData = nullptr;
    }

    char* ZEngineDataBase::GetStaticBuffer()
    {
        return reinterpret_cast<char*>(m_pStaticBuffer);
    }

    char* ZEngineDataBase::GetAnimBuffer()
    {
        return reinterpret_cast<char*>(m_pPackedAnims);
    }

    void ZEngineDataBase::CreateGeoms(REFTAB* prtCreatedGeoms, ZStackArray<1000, SMakeGeomDynamic>* pMakeDynArray, const char* pGeomsData, const char* pStaticBuffer, IInputStream& property_in_stream)
    {
        int32_t lNumberOfPackedGeoms = 0;

        PUSH_MEMORY_COLOR(0x606060u);
        MarkNonRunTime();

        // TODO: Finish me

        if (g_pSysInterface->GetOption("PrintEngineInfo", nullptr))
        {
            ZINFO("Number of Packed Geoms: %d", lNumberOfPackedGeoms);
        }

        // TODO: Finish me

        if (g_pSysInterface->GetOption("PrintEngineInfo", nullptr))
        {
            ZINFO("Highest used geomnumber: %d", 123); // TODO: Replace '123' to struct read
        }

        // TODO: Finish me

        if (!m_pRoot)
        {
            m_pRoot = AllocRootGroup();
            m_pRoot->MakeDynamicContainer(true);
        }
        ZASSERT(m_pRoot);

        // TODO: Finish me

        ZMessageResolver::ResolveAll();

        // TODO: Finish me

        if (!g_pSysInterface->m_bDisableLight)
        {
            // TODO: Finish me
        }

        // TODO: Finish me
        // InitResourceGeoms(...);
        // BS_Runtime::ZMaterialDescriptionDB::m_Instance->RemapGeoms(...);
    }

    void ZEngineDataBase::LoadProperties(uint32_t lNrPackedGeoms, ZBaseGeom** BaseGeoms, IInputSerializerStream& in)
    {
        if (lNrPackedGeoms)
        {
            ZBaseGeom** pBegin = BaseGeoms;
            ZBaseGeom** pEnd = BaseGeoms;

            LoadPropertiesRecursive(in, pBegin, m_pRoot->BaseGeom());
            const auto n_geoms = pBegin - pEnd;
            ZASSERT(n_geoms == lNrPackedGeoms); // Check count of visited objects
        }
    }

    void ZEngineDataBase::LoadPropertiesRecursive(IInputSerializerStream& in, ZBaseGeom**& BaseGeoms, ZBaseGeom* pObject)
    {
        ++g_iLoadPropertiesProgress;

        if (!(g_iLoadPropertiesProgress % 32))
        {
            ZASSERT(g_iLoadPropertiesTotal > 0);

            if (g_iLoadPropertiesTotal > 0)
            {
                // Recalc fake progress
                float fNewProgress = (g_iLoadPropertiesProgress * 0.7f) / (g_iLoadPropertiesTotal + 0.2f);
                if (fNewProgress >= 1.0f)
                {
                    fNewProgress = 1.0f;
                }

                SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_GEOMS, nullptr, fNewProgress);
            }
        }


        const char* pszName = pObject->Name();
        in.Exchange<ZGEOM>(pszName, *pObject->GetGeom());

        static ZTokenCache ControllerToken { "Controllers" };
        in.GetToken(&ControllerToken);

        uint32_t lControllersNr = 0;
        in.ExchangeContainer(ControllerToken, lControllersNr);

        for (int i = 0; i < lControllersNr; ++i)
        {
            static ZTokenCache ControllerNameToken { "ControllerName" };

            in.GetToken(&ControllerNameToken);
            const char* pszEventName;
            in.ExchangeData(pszEventName);

            auto* pEvent = pObject->GetGeom()->AddEvent(pszEventName);
            if (pEvent)
            {
                ZASSERT(pEvent->m_pBaseGeom);
                in.Exchange<ZBaseConRout>(ZToken::Void, *pEvent);
                ZASSERT(pEvent->m_pBaseGeom);

                auto* pRout = static_cast<ZBaseConRout*>(pEvent);
                ++const_cast<ZROUTCLASSINFO*>(pRout->m_pRoutClassInfo)->m_lSceneInstanceCount;
                pEvent->RegisterInstance();

                ZASSERT(pEvent->m_pBaseGeom);
            }
            else
            {
                ZWARN2("ZEngineDataBase::LoadProperiesRecursive: Skipped event '%s' of object '%s' due required event not found", pszEventName, pszName);
                in.SkipObject();
            }
        }

        uint32_t lChildNr = 0;
        in.ExchangeContainer("Children", lChildNr);

        if (lChildNr > 0)
        {
            ZASSERT(pObject->IsDerivedFrom<ZGROUP>());

            for (int i = 0; i < lChildNr; ++i)
            {
                auto* pChild = BaseGeoms[i + 1];
                ZASSERT(INEDITOR || pChild->Parent() == pObject);

                LoadPropertiesRecursive(in, BaseGeoms, pChild);
            }
        }
    }

    void ZEngineDataBase::SetSaveObject(ZSaveClass* pSaveObj)
    {
        m_pSaveObject = pSaveObj;
    }

    void ZEngineDataBase::LoadZDefines(IInputSerializerStream& stream)
    {
        uint32_t lDefinesNr;
        stream.ExchangeContainer("ZDefines", lDefinesNr);

        for (int i = 0; i < lDefinesNr; ++i)
        {
            const char* pszName;
            uint32_t lType;

            stream.Exchange("Name", pszName);
            stream.Exchange("Type", lType);

            switch (lType)
            {
                case 2: // INT32
                {
                    int lSize;
                    int lData;

                    stream.Exchange("Size", lSize);
                    stream.Exchange("Data", lData);

                    m_SceneCom.SetVal(pszName, &lData, lSize, CCOMType::CCOM_TYPE_INT32);
                }
                break;

                case 3: // FLOAT
                {
                    int lSize;
                    int lData;

                    stream.Exchange("Size", lSize);
                    stream.Exchange("Data", lData);

                    m_SceneCom.SetVal(pszName, &lData, lSize, CCOMType::CCOM_TYPE_FLOAT);
                }
                break;

                case 12: // STRING
                {
                    int lData;
                    stream.Exchange("Data", lData);

                    m_SceneCom.SetVal(pszName, &lData, CCOMType::CCOM_TYPE_STRING);
                }
                break;

                case 14: // FILE
                {
                    int lData;
                    stream.Exchange("Data", lData);

                    m_SceneCom.SetVal(pszName, &lData, CCOMType::CCOM_TYPE_FILE);
                }
                break;

                case 16: // GEOMREF
                {
                    int lData;
                    stream.Exchange("Data", lData);

                    m_SceneCom.SetVal(pszName, &lData, CCOMType::CCOM_TYPE_GEOMREF);
                }
                break;

                case 17: // GEOMREFTAB
                {
                    REFTAB32 aCollected;
                    uint32_t lEntriesNr = 0;

                    stream.ExchangeContainer(pszName, lEntriesNr);
                    for (int j = 0; j < lEntriesNr; ++j)
                    {
                        const char* pszGeomName;
                        stream.ExchangeData(pszGeomName);

                        ZREF rGeom = GetREFByName(pszGeomName);
                        aCollected.Add(rGeom);
                    }

                    m_SceneCom.SetVal(pszName, &aCollected, CCOMType::CCOM_TYPE_GEOMREFTAB);
                }
                break;

                default:
                    ZASSERT(false);
                    break;
            }
        }
    }

    uint32_t ZEngineDataBase::GetSoundGraphSize()
    {
        MYSTR sGeomsFile = CalcCacheFileName(m_FileName, "sgp");
        return g_pSysFile->GetSize(sGeomsFile, false);
    }

    void ZEngineDataBase::GetSoundGraphData(void* pData, uint32_t lSize)
    {
        MYSTR sGeomsFile = CalcCacheFileName(m_FileName, "sgp");
        g_pSysFile->Load(sGeomsFile, pData, lSize, 0, false);
    }
}
