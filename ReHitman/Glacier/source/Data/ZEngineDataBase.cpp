#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZLoadGameInfoBase.h>
#include <Glacier/Data/ZStaticGameLevelData.h>
#include <Glacier/ZPackedDataChunk.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/ResourceCollection.h>
#include <Glacier/Geom/ZEngineGeomControl.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/System/CConfiguration.h>
#include <Glacier/System/ZSysMem.h>
#include <Glacier/System/ZDllBase.h>
#include <Glacier/Com/Globals.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Com/CGlobalCom.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/Prim/SPrimObjectScatter.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/Debug/Globals.h>
#include <Glacier/Render/Debug/ZDrawDebugTimer.h>
#include <Glacier/Render/ZWaterManager.h>
#include <Glacier/Debug/ZMemReadOut.h>
#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Serializer/ZIOInputStream.h>
#include <Glacier/Serializer/ZInputStream.h>
#include <Glacier/Serializer/ZPackedInput.h>
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
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/Geom/ZGeomListTypeUtils.h>
#include <Glacier/Data/SPackedGeomsTree.h>
#include <Glacier/Data/SCompiledGeom.h>
#include <Glacier/Audio/ZSoundDllBase.h>
#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/Materials/BS_Runtime.h>
#include <Glacier/Animation/Manager.h>
#include <Glacier/ZSTL/CHUNKFILE.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Action/ActionInterface.h>
#include <Glacier/ZSTL/ZPoolAllocRefTab.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/PF4/ZData.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Input/SysInput.h>
#include <Glacier/Input/ZInterface.h>
#include <Glacier/Materials/BS_Runtime.h>
#include <Glacier/Debug/ZPushMemColor.h>
#include <Glacier/Debug/ZMemReadOut.h>
#include <Glacier/LoaderSequence/ZLoader_Sequence_Player.h>
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

        // A single record in ZGameData's total/used/big weapon prim buffers.
        // The stride is 3 dwords: m_lPrim is the prim id, and the "total" list
        // additionally carries a two-dword weapon hash in m_lHash that is matched
        // against the "WeaponHashes" global COM value. The "used" and "big" lists
        // only consume m_lPrim.
        //
        struct SWeaponPrimEntry
        {
            uint32_t m_lPrim;
            uint32_t m_lHash[2];
        };
        RE_VERIFY_SIZE(SWeaponPrimEntry, 0xC);
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
            auto token = stream.GetToken("Sound");
            stream.Exchange<ZGEOM>(token, g_pSysInterface->GetSoundDll()->m_pPlayer);
        }

        {
            auto token = stream.GetToken("GeomBuffer");
            stream.Exchange<ZGeomBuffer>(token, *m_pGeomBuffer);
        }

        g_pRenderDll->ExchangeObject(stream);

        {
            auto token = stream.GetToken("Root");
            stream.Exchange<ZROOM>(token, *m_pRoot);
        }

        g_pGameData->LoadSave(stream, m_SavingGame);
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
            ZLoader_Sequence_Player::Set_Progress(fProgressValue);
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
        // PC 0x45FCF0. Master level-load orchestrator. The prologue and the
        // content buffers below are complete; the remaining pipeline (geoms data
        // load + zgf unzip, prim/material buffer install, CreateGeoms, tree
        // build, water manager and post-init stages) is still pending.

        // ---- Prologue ----
        PUSH_MEMORY_COLOR(0x80FF80u);

        g_pSysInterface->ResetClassInstanceCount();
        g_pSysInterface->Reset_TimeMultiplier_Lock();
        g_pSysInterface->Set_TimeMultiplier(1.0f);

        if (g_pGameData)
            g_pGameData->OnLevelChangeBegin();

        // A saved-game control stream means this level load is restoring a save game.
        // pControlStream is retained for the load-from-save restore stage below.
        IInputStream* pControlStream = ZLoadGameInfoBase::CreateControlStream();
        if (pControlStream)
            m_LoadingGame = true;

        m_bPause = false;
        PauseScene(false);
        g_pSysInterface->InitActionMap();
        g_pSysInterface->GetConfiguration()->Apply();
        ZEngineGeomControl::GetInstance().SetChangeDetection(false);
        ZCollisionBase::InitCollision(false);

        // ---- Content buffer sizes ----
        const uint32_t lPrimsSize   = GetPrimsSize();
        const uint32_t lTextureSize = GetTextureSize();
        const uint32_t lMatSize     = GetMaterialsSize();
        const uint32_t lStaticSize  = GetStaticSize();

        SetAllocSequencePercent(AS_DLCLOAD, nullptr, 0.0f);

        // PC __debugbreak()s when a packed data size is negative ("Scene packed
        // data undefined", "File does not exist?").
        ZASSERT(static_cast<int32_t>(lTextureSize) >= 0 &&
                static_cast<int32_t>(lPrimsSize) >= 0 &&
                static_cast<int32_t>(lStaticSize) >= 0);

        // ---- Texture buffer ----
        InstallTextureBuffer();

        // ---- Materials buffer ----
        void* pMaterialsData = nullptr;
        if (lMatSize > 0)
            pMaterialsData = ZUniMemory::Allocate(lMatSize + 0x20000);
        GetMaterialsData(pMaterialsData, lMatSize);
        g_pRenderDll->InstallMaterialBuffer(pMaterialsData, lMatSize, lMatSize + 0x20000);

        // ---- Prims buffer ----
        // The +0x60000 slack is the workspace CompactPrimBuffer later compacts into.
        uint32_t lPrimsBufferSize = 0;
        void* pPrimsData = nullptr;
        {
            PUSH_MEMORY_COLOR(0xE0E000u);
            lPrimsBufferSize = lPrimsSize + 0x60000;
            pPrimsData = ZUniMemory::Allocate(lPrimsBufferSize);
        }

        // ---- Static buffer ----
        {
            PUSH_MEMORY_COLOR(0xFF0080u);
            auto* pStaticData = static_cast<uint8_t*>(ZUniMemory::Allocate(lStaticSize));
            GetStaticData(pStaticData, lStaticSize);
            m_pStaticBuffer = pStaticData;
            m_lStaticBufferLength = static_cast<int>(lStaticSize);
        }

        GetPrimsData(pPrimsData, lPrimsSize);

        // ---- Locale resources ----
        if (m_pLocaleResources)
        {
            MYSTR sLocaleFile = CalcCacheFileName(m_FileName, "loc");
            m_pLocaleResources->LoadFile(sLocaleFile);
        }

        // ---- Sound data ----
        // The sound data buffer is retained for the later InstallSounds stage.
        void* pSoundData = nullptr;
        uint32_t lSoundDataSize = 0;
        if (g_pSysInterface->m_pSoundDll)
        {
            lSoundDataSize = GetSoundDataSize();
            if (lSoundDataSize != static_cast<uint32_t>(-1))
            {
                PUSH_MEMORY_COLOR(0xFF7777u);
                pSoundData = ZUniMemory::Allocate(lSoundDataSize);
                GetSoundData(pSoundData, lSoundDataSize);
            }
        }

        // ---- Anim data ----
        const uint32_t lAnimsSize = GetAnimsSize();
        if (lAnimsSize > 0)
        {
            PUSH_MEMORY_COLOR(0x808040u);
            m_pPackedAnims = static_cast<CHUNKFILE*>(ZUniMemory::Allocate(lAnimsSize));
            m_lPackedAnimsLength = lAnimsSize;
            GetAnimsData(m_pPackedAnims, lAnimsSize);
        }

        // ---- Trees, sound graph and packed static game level data ----
        LoadRoomTrees();
        LoadBoundTrees();
        LoadSoundGraph();
        CreatePackedStaticGameLevelData();

        // ---- Geoms data (packed GMS) ----
        // Aligned buffer holds the packed GMS; it is decompressed further down
        // (after the InstallSounds stage) into the block CreateGeoms consumes.
        const uint32_t lGeomsSize = GetGeomsSize();
        char* pGeomsData = static_cast<char*>(ZUniMemory::Allocate((lGeomsSize + 15) & 0xFFFFFFF0));
        GetGeomsData(pGeomsData, lGeomsSize);

        // ---- GeomFiles (zgf) big chunk ----
        // A separate resource pack unzipped through the render draw allocator and
        // registered as a "big" file the loaders read. It stays loaded until the
        // final cleanup stage (RemoveBig + IDraw::Free). packedChunk is reused for
        // the GMS decompression below.
        const uint32_t lGeomFilesSize = GetGeomFilesSize();
        ZPackedDataChunk packedChunk;
        void* pGeomFilesBig = nullptr;
        int lGeomFilesUnpackedSize = 0;
        if (static_cast<int32_t>(lGeomFilesSize) <= 0)
        {
            g_pSysFile->RemoveAllBigs();
            IDraw::Instance()->InitAllocation();
        }
        else
        {
            auto* pZgfData = static_cast<uint32_t*>(ZUniMemory::Allocate((lGeomFilesSize + 15) & 0xFFFFFFF0));
            GetGeomFilesData(pZgfData, lGeomFilesSize);

            packedChunk.m_bCustomBuffer = true;
            packedChunk.m_pPackedData = reinterpret_cast<uint8_t*>(pZgfData);
            packedChunk.m_iRawDataLength = static_cast<int>(pZgfData[0]);
            packedChunk.m_iPackedDataLength = static_cast<int>(pZgfData[1]);
            packedChunk.m_eCompression =
                (*reinterpret_cast<const uint8_t*>(pZgfData + 2) != 0)
                    ? ZPackedDataChunk::UNCOMPRESSED
                    : ZPackedDataChunk::ZIPPED;

            const uint32_t lUnpackedAligned =
                (static_cast<uint32_t>(packedChunk.m_iRawDataLength) + 15) & 0xFFFFFFF0;

            IDraw::Instance()->InitAllocation();
            pGeomFilesBig = IDraw::Instance()->Alloc(static_cast<int>(lUnpackedAligned), __FILE__, __LINE__);
            packedChunk.unzip(static_cast<char*>(pGeomFilesBig), packedChunk.m_iRawDataLength);
            ZUniMemory::Free(pZgfData);

            lGeomFilesUnpackedSize = packedChunk.m_iRawDataLength;
            g_pSysFile->UseBig(static_cast<CHUNKFILE*>(pGeomFilesBig), "GeomFiles");
        }

        // NOTE: InstallSounds (ZDllSound data/stream/whd/wav init) belongs before
        // the GMS decompress below; it is left pending and does not feed pGmsData.

        // ---- Decompress the packed GMS into the CreateGeoms input buffer ----
        // Reuses packedChunk; the buffer is allocated in the backward-growing
        // memory region (matches PC) so it interleaves with the level data.
        packedChunk.m_bCustomBuffer = true;
        packedChunk.m_pPackedData = reinterpret_cast<uint8_t*>(pGeomsData);
        packedChunk.m_iRawDataLength = static_cast<int>(*reinterpret_cast<const uint32_t*>(pGeomsData));
        packedChunk.m_iPackedDataLength = static_cast<int>(reinterpret_cast<const uint32_t*>(pGeomsData)[1]);
        packedChunk.m_eCompression =
            (*reinterpret_cast<const uint8_t*>(pGeomsData + 8) != 0)
                ? ZPackedDataChunk::UNCOMPRESSED
                : ZPackedDataChunk::ZIPPED;

        if (ISysMem::Exists())
            ISysMem::Instance().SetAllocDirection(ISysMem::AD_BACKWARD);
        char* pGmsData = static_cast<char*>(
            ZUniMemory::Allocate((static_cast<uint32_t>(packedChunk.m_iRawDataLength) + 15) & 0xFFFFFFF0));
        if (ISysMem::Exists())
            ISysMem::Instance().SetAllocDirection(ISysMem::AD_FORWARD);
        packedChunk.unzip(pGmsData, packedChunk.m_iRawDataLength);
        ZUniMemory::Free(pGeomsData);

        // The decompressed GMS block begins with its packed-geoms header; the
        // remaining stages index it through named offset fields.
        const auto* pGms = reinterpret_cast<const SPackedGeomsHeader*>(pGmsData);

        // ---- Weapon prims / excluded anim names ----
        // Both offsets point into the static buffer where those tables were packed.
        const uint32_t lWeaponPrimsOffset = pGms->m_iWeaponPrimsOffset;
        if (lWeaponPrimsOffset && g_pGameData)
            g_pGameData->InitWeaponHandles(
                reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(m_pStaticBuffer) + lWeaponPrimsOffset));
        const uint32_t lExcludedAnimOffset = pGms->m_iExcludedAnimNamesOffset;
        if (lExcludedAnimOffset && g_pGameData)
            g_pGameData->InitExcludedAnimNames(
                reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(m_pStaticBuffer) + lExcludedAnimOffset));

        // ---- Prim control buffers ----
        g_pRenderDll->CreatePrimControl();

        // A mission (not the main menu / loader) prunes the leftover weapon prims
        // left over from the menu before compacting. PC uses a case-sensitive
        // compare against "_main"; the weapon-handles offset being present is the
        // same +0x40 header field read above.
        if (lWeaponPrimsOffset && strcmp(g_pSysInterface->m_sDefaultScene.String, "_main") != 0)
            PurgePrimBuffer();

        // CompactPrimBuffer reclaims freed prims in place and returns the new
        // packed size; the buffer is installed with the +0x60000 workspace slack
        // added back.
        const uint32_t lCompactedSize = g_pRenderDll->CompactPrimBuffer(pPrimsData, lPrimsSize);
        g_pRenderDll->InstallPrimBuffer(pPrimsData, lCompactedSize + 0x60000);

        // Hand the (now compacted) prim region to the allocator so it can release
        // any trailing pages it no longer needs (PC asserts ISysMem is present).
        ZASSERT(ISysMem::Exists());
        char* pPrimRegion = static_cast<char*>(pPrimsData);
        uint32_t lPrimRegionSize = lCompactedSize + 0x60000;
        ISysMem::Instance().Shrink(pPrimRegion, lPrimRegionSize);

        // ---- Scatter-prim region tinting (debug visualization) ----
        // Sum the total byte size of every packed scatter prim (lType == PTOBJECTSCATTER,
        // lPackType == 1) and tint that many bytes at the tail of the prim buffer.
        // The per-prim size is the dword at +0x58 of SPrimObjectScatter (PC PAD58 /
        // PS2 dword index 22). Guarded on the debug ZMemReadOut singleton existing.
        if (ZMemReadOut::Exists())
        {
            const uint32_t lNrPrims = *reinterpret_cast<const uint32_t*>(static_cast<char*>(pPrimsData) + 4);
            uint32_t lScatterPrimSize = 0;
            for (uint32_t i = 0; i < lNrPrims; ++i)
            {
                const auto* pScatter = static_cast<const SPrimObjectScatter*>(g_apPrimHandleToPointerTable[i]);
                if (pScatter && pScatter->lType == PTOBJECTSCATTER && pScatter->lPackType == 1)
                    lScatterPrimSize += *reinterpret_cast<const uint32_t*>(&pScatter->fDrawDist);
            }
            ZMemReadOut::Instance().OverrideMemColors(
                static_cast<char*>(pPrimsData) + (lPrimRegionSize - lScatterPrimSize),
                lScatterPrimSize, 0xF0F000u);
        }

        // ---- Material description database ----
        // Created fresh per level, then initialized from the packed material byte
        // stream. The stream offset (SPackedGeomsHeader::m_iMaterialDescOffset,
        // +0x30) is relative to the static buffer (matches PC/PS2).
        BS_Runtime::ZMaterialDescriptionDB::Create();
        BS_Runtime::ZMaterialDescriptionDB::Instance().Init(
            m_pStaticBuffer + pGms->m_iMaterialDescOffset);

        // ---- Animation manager ----
        // Resolved from chunk 4 of the packed ANM file (m_pPackedAnims) when present.
        if (m_pPackedAnims)
        {
            if (CHUNKFILE* pAnimChunk = m_pPackedAnims->FindChild(4))
                m_AnimationManager = Animation::Manager::CreateFromDataBlock(pAnimChunk->Data(), pAnimChunk->DataSize());
        }

        // ---- Pathfinder4 data ----
        // Offset (SPackedGeomsHeader::m_lOffsetPathfinder4Data, +0x34) is relative
        // to the decompressed GMS buffer.
        const uint32_t lPathfinderOffset = pGms->m_lOffsetPathfinder4Data;
        if (lPathfinderOffset)
            InitPathfinder4Data(pGmsData + lPathfinderOffset);

        // ---- Physics data ----
        // Offset (SPackedGeomsHeader::m_lPhysicsDataOffset, +0x38) is relative to the
        // static buffer; -1 means no physics data. PC breaks when init reports failure.
        const uint32_t lPhysicsOffset = pGms->m_lPhysicsDataOffset;
        if (static_cast<int32_t>(lPhysicsOffset) != -1)
        {
            if (!InitPhysicsData(reinterpret_cast<const char*>(m_pStaticBuffer) + lPhysicsOffset))
                ZASSERT(false);
        }

        // ---- CreateGeoms ----
        // Unlock refs and raise the min/max lock so the geom construction can
        // adjust bounds without triggering updates, then hook the missing-only
        // initializer. PC re-runs InitWeaponHandles here (same +0x40 offset into
        // the static buffer) after the lock count is raised.
        g_pSysInterface->UnlockRefs();
        LockMinMax();
        PackHookMissingOnlyInitialize();

        if (lWeaponPrimsOffset && g_pGameData)
            g_pGameData->InitWeaponHandles(
                reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(m_pStaticBuffer) + lWeaponPrimsOffset));

        REFTAB32 rtCreatedGeoms;
        ZStackArray<1000, SMakeGeomDynamic> makeDynArray;
        IInputStream* pPropertyStream = CreatePropertyInputStream();

        CreateGeoms(&rtCreatedGeoms, &makeDynArray, pGmsData,
                    reinterpret_cast<const char*>(m_pStaticBuffer), *pPropertyStream);

        // PC releases the property stream right after use (vtable[0] = deleting destructor).
        if (pPropertyStream)
            ZUniMemory::Delete(pPropertyStream);

        // ---- Post-CreateGeoms cleanup ----
        CleanupPropertyData();
        g_pSysFile->RemoveAllBigs();
        // The decompressed GMS block is no longer needed once the geoms exist.
        ZUniMemory::Free(pGmsData);

        // If the first render window has no camera yet, give it a default one so
        // the scene is viewable. PC: WindowFirst->GetCamera(0) == 0 ->
        // CreateDefaultCam(0) -> WindowFirst->AddCamera(pCam, 0, 0).
        if (!g_pSysInterface->WindowFirst->GetCamera(0))
        {
            ZCAMERA* pDefaultCam = CreateDefaultCam(nullptr);
            if (pDefaultCam)
                g_pSysInterface->WindowFirst->AddCamera(pDefaultCam, 0, 0.0f);
        }

        // ---- Tree build and deferred geom setup ----
        CheckAndMakeStaticContainer();
        g_pSysInterface->ClearTime();
        UnlockMinMax();
        CalcAllMinMax();

        MakeDynamicGeomsDynamic(&makeDynArray);
        CreateRoomTrees();
        CreateBoundTrees();
        if (m_pRoot)
            m_pRoot->CreateDynamicTrees();
        MakeAutoAssignGeomsAutoAssign(&makeDynArray);
        ClearSaveLoadFlags();

        // ---- Water manager ----
        // Count ZSTDOBJ-derived geoms whose prim is a water patch (PTWATERPATCH == 11)
        // and create the water manager sized to that count.
        g_pWaterManager = nullptr;
        {
            int lWaterPatchCount = 0;
            for (auto* pBaseGeom = m_pRoot->BaseGeom(); pBaseGeom; m_pRoot->RecurGetNext(&pBaseGeom))
            {
                if (!pBaseGeom->IsDerivedFrom<ZSTDOBJ>())
                    continue;

                const ZPrimHandle hPrim { pBaseGeom->Prim() };
                const SPrimHeader* pHdr = hPrim;

                if (pHdr && pHdr->lType == PTWATERPATCH)
                    ++lWaterPatchCount;
            }

            if (lWaterPatchCount > 0)
            {
                g_pWaterManager = ZUniMemory::New<ZWaterManager>(lWaterPatchCount);
            }
        }

        // ---- Runtime flags and level-change callbacks ----
        MarkRunTime();
        ZEngineGeomControl::GetInstance().SetChangeDetection(true);
        if (g_pGameData)
        {
            g_pGameData->OnLevelChangeLoadDone();
        }

        CreateSoundGraph();

        // ---- GeomFiles big cleanup ----
        if (static_cast<int32_t>(lGeomFilesSize) > 0)
        {
            g_pSysFile->RemoveBig("GeomFiles");
            IDraw::Instance()->Free(pGeomFilesBig, lGeomFilesUnpackedSize);
        }

        // ---- Init (STATUS_Init / STATUS_Init2 passes) ----
        Init();

        // TODO: Finish me after ZSaveMemoryManagerFake + ZEngineDataCRCInputStream
        //   reversed: the saved-game restore block (ZCompressedInputStream +
        //   ZInputStream + ZPackedInput chain + ISerializerStream::Exchange +
        //   UpdateMovedGeoms) inside the if (m_LoadingGame) branch.
        // Per-render-window draw-buffer (re)allocation now that the scene geoms
        // exist. PC: WindowFirst vtbl +14 = ZRenderX86::AllocateDrawBuffers.
        for (auto* pCurrentRender = g_pSysInterface->WindowFirst; pCurrentRender; pCurrentRender = pCurrentRender->Nxt)
        {
            pCurrentRender->AllocateDrawBuffers();
        }
        // TODO: Finish me: sub_45AC30 + SetAllocSequencePercent(AS_GEOMS, ..., 1.0).

        LockMinMax();

        // ---- PostInit inline pass (STATUS_PostInit) ----
        ZGEOM::m_PreferedStatus = ZGEOM::STATUS_PostInit;
        g_pEngineData->m_pRoot->DoInit();
        ZEventBase::m_DefaultStatus = ZEventBase::STATUS_PostInit;
        m_EventList.DoInit();

        m_LoadingGame = false;
        if (g_pSysInterface->m_pSoundDll)
            g_pSysInterface->m_pSoundDll->CleanupBeforeCloseDown();

        g_pSysInterface->LockRefs();

        if (SysInput::instance)
        {
            SysInput::instance->Update();
            SysInput::instance->ResetTables(true);
        }

        if (g_pGameData)
        {
            g_pGameData->OnLevelChangeFinish();
        }

        // TODO: Finish me - remaining AllocSequence tail (PC order):
        //   InstallSounds (ZDllSound) still belongs before the GMS decompress.
        //   Default-cam / global strip colli tree (render window vtbl +36/+38).

        ZLoadGameInfoBase::Destroy();
        DEBUG_WhenToPrintMemory = g_pSysInterface->m_lFrameCount + 10;
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

        auto* pCurrentGeomType = &pGeomTypeCount;
        for (uint32_t i = 0; i < lNrGeomTypes; ++i, ++pCurrentGeomType)
        {
            lNrBaseGeoms += pCurrentGeomType->m_lGeomCount;

            const auto* pClassInfo = ZGEOM::GetFactory().Find(pCurrentGeomType->m_lGeomType);
            const uint32_t lExtraGeomSize = pClassInfo ? pClassInfo->Size() : sizeof(ZGEOM);
            ZASSERT((lExtraGeomSize & 3) == 0);

            lExtraGeomsSize += lExtraGeomSize * pCurrentGeomType->m_lGeomCount;
            if (!ForceExtraGeom())
            {
                ZASSERT(pCurrentGeomType->m_lNoNeedExtraGeom <= pCurrentGeomType->m_lGeomCount);
                lExtraGeomsSize -= lExtraGeomSize * pCurrentGeomType->m_lNoNeedExtraGeom;
            }
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

        for (; *reinterpret_cast<int32_t*>(pBufferStart) != -1; )
        {
            pBufferStart = ZCollisionBase::GetCollisionInterface()->LoadInternColiTree(pBufferStart);
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
        if (!g_pSysInterface->m_pSoundDll)
            return;

        PUSH_MEMORY_COLOR(0x7777u);
        g_pSysInterface->GetSoundDll()->InitializeSoundGraph();
    }

    void ZEngineDataBase::LoadSoundGraph()
    {
        if (!g_pSysInterface->m_pSoundDll) return;

        PUSH_MEMORY_COLOR(0x7777u);

        uint32_t lSize = GetSoundGraphSize();
        if (lSize > 0)
        {
            auto* pData = static_cast<char*>(ZUniMemory::Allocate(lSize));
            GetSoundGraphData(pData, lSize);
            g_pSysInterface->GetSoundDll()->InstallSoundGraph(pData, lSize);
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
        ZASSERT(g_pRenderDll);
        if (!g_pRenderDll)
            return false;

        g_pSysInterface->WindowFirst = g_pRenderDll->SetupWindow(g_pSysInterface->MainhWnd);

        MYSTR sInitialScene = m_FileName;
        if (char* pSeparator = std::strchr(sInitialScene.String, ';'))
        {
            m_FileName.SetString(pSeparator + 1);
            *pSeparator = '\0';
        }

        if (g_pSysInterface->m_pSoundDll)
        {
            PUSH_MEMORY_COLOR(0xFFFFC0u);
            g_pSysInterface->GetSoundDll()->Initialize();
        }

        LoadScene(sInitialScene);
        ControlSceneChange();
        return true;
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
        g_pSysInterface->UnlockRefs();
        if (g_pSysInterface->WindowFirst)
        {
            IDraw::Instance()->Flush();
        }

        if (ZCollisionBase::s_pCollisionBase)
        {
            ZCollisionBase::s_pCollisionBase->FreeSceneMemory();
        }

        BS_Runtime::ZMaterialDescriptionDB::Destroy();

        if (m_pRoomColiTreeData)
        {
            ZUniMemory::Free(m_pRoomColiTreeData);
            m_pRoomColiTreeData = nullptr;
        }

        if (m_pGlobalColiTreeData)
        {
            ZUniMemory::Free(m_pGlobalColiTreeData);
            m_pGlobalColiTreeData = nullptr;
        }

        if (m_pEntityTracker)
        {
            ZUniMemory::Delete(m_pEntityTracker);
            m_pEntityTracker = nullptr;
        }

        if (m_pPathfinder4Data)
        {
            ZUniMemory::Delete(m_pPathfinder4Data);
            m_pPathfinder4Data = nullptr;
        }

        if (m_AnimationManager)
        {
            m_AnimationManager->Clear();
            Animation::instance = nullptr;
            ZUniMemory::Delete(m_AnimationManager);
        }

        m_AnimationManager = nullptr;

        if (m_pPackedTreeData)
        {
            ZUniMemory::Free(m_pPackedTreeData);
            m_pPackedTreeData = nullptr;
        }

        m_lPackedTreeDataLength = 0;
        DeleteAllGeoms();

        m_EventList.Clear();

        if (m_pScheduledUpdate)
        {
            ZUniMemory::Delete(m_pScheduledUpdate);
            m_pScheduledUpdate = nullptr;
        }

        if (auto* pInst = ZStaticGameLevelData::Instance())
        {
            pInst->Destroy();
        }

        ZEngineGeomControl::GetInstance().Clear();

        if (m_pListUser)
        {
            ZUniMemory::Delete(m_pListUser);
            m_pListUser = nullptr;
        }

        m_SceneCom.Clear();

        if (m_pStaticBuffer)
        {
            ZUniMemory::Free(m_pStaticBuffer);
            m_pStaticBuffer = nullptr;
        }

        if (m_pPackedAnims)
        {
            ZUniMemory::Free(m_pPackedAnims);
            m_pPackedAnims = nullptr;
            m_lPackedAnimsLength = 0;
        }

        for (auto* pCurrentRender = g_pSysInterface->WindowFirst; pCurrentRender; pCurrentRender = pCurrentRender->Nxt)
        {
            pCurrentRender->RemoveCameras();
            pCurrentRender->FreeDrawBuffers();
        }

        Action::Free();

        FreeMsgValues();

        --m_lLockMinMax;
        if (m_bRunTime)
        {
            m_bRunTime = false;
        }

        g_pSysInterface->LockRefs();

        if (g_pGameDataFactory)
        {
            g_pGameDataFactory->DestroyGameData();
        }
    }

    // Parks the currently loaded scene into pScene and detaches it from the engine so a
    // new scene can be loaded on top of it (the dual-scene stack). Only scene-owned
    // buffers are stashed; global engine databases are left intact. Symmetric with PushValues.
    void ZEngineDataBase::FreeSceneMemory(ZScene* pScene)
    {
        pScene->_lLockMinMax = m_lLockMinMax;
        ++m_lSceneDepth;
        m_lLockMinMax = 0;
        m_rParticleControllerGeom = 0;

        if (g_pRenderDll)
        {
            g_pRenderDll->PushScene("");
        }

        if (m_pRoot)
        {
            pScene->_pBigFiles = nullptr;

            pScene->_pStaticBuffer = m_pStaticBuffer;
            m_pStaticBuffer = nullptr;

            pScene->_lStaticBufferLength = m_lStaticBufferLength;
            m_lStaticBufferLength = 0;

            pScene->_pPackedAnims = m_pPackedAnims;
            m_pPackedAnims = nullptr;

            pScene->_lPackedAnimsLength = static_cast<int>(m_lPackedAnimsLength);
            m_lPackedAnimsLength = 0;

            pScene->_pGeomBuffer = m_pGeomBuffer;
            m_pGeomBuffer = nullptr;

            pScene->_pPackedTreeData = m_pPackedTreeData;
            m_pPackedTreeData = nullptr;

            pScene->_pRoot = m_pRoot;

            g_pSysInterface->WindowFirst->RemoveCameras();
            m_pRoot = nullptr;

            pScene->_FrameTime = g_pSysInterface->FrameTime;
            pScene->_PreFrameTime = g_pSysInterface->PreFrameTime;
            pScene->_ActTime = g_pSysInterface->m_fActualTime;
            g_pSysInterface->ResetTime();

            // Reset the message-name table and hash. The hash pointer is cleared before
            // FreeMsgValues so the hash object is not freed here (it is released with the pool).
            for (auto& entry : m_ZMsgStrings)
            {
                entry = {};
            }
            m_iNumRegisteredMessages = 0;
            m_pZMessageHash = nullptr;

            FreeMsgValues();
        }
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

        g_pSysInterface->FrameTime = pNewScene->_FrameTime;
        g_pSysInterface->PreFrameTime = pNewScene->_PreFrameTime;
        g_pSysInterface->ResetTime();
        g_pSysInterface->m_fActualTime = pNewScene->_ActTime;
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

    ZREF ZEngineDataBase::GetSceneVar(const char* varname)
    {
        int32_t lValue = 0;
        m_SceneCom.GetVal(varname, &lValue);

        return static_cast<ZREF>(lValue);
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
        if (g_pGameDataFactory)
            g_pGameDataFactory->CreateGameData();

        FreeSceneMemory(m_pScene);

        MYSTR sSceneName(m_pScene->GetSceneName());
        if (striwcmp(sSceneName, "*premission*"))
            g_pSysInterface->m_lTextureResolution[0] = g_pSysInterface->m_lTextureResolution[1];
        else
            g_pSysInterface->m_lTextureResolution[0] = 0;

        MYSTR sScenesPath = g_pSysInterface->ScenesPath();
        if (!std::strchr(sSceneName.String, '.'))
        {
            sSceneName += MYSTR(stricmpend(g_pSysInterface->m_sDefaultScene, ".zip") ? ".gms" : ".zip");
        }

        if (stricmpend(sSceneName, ".gms"))
        {
            if (sSceneName.Length() > 0 && !std::strchr(sSceneName.String, ':'))
                m_FileName = sScenesPath + MYSTR("\\") + sSceneName;
            else
                m_FileName = sSceneName;

            sSceneName = m_FileName;
        }
        else
        {
            if (std::strchr(sSceneName.String, ':') || sScenesPath.Length() == 0)
            {
                m_FileName = sSceneName;
            }
            else if (_memicmp(sSceneName, sScenesPath, sScenesPath.Length()))
            {
                const char* pSeparator = sScenesPath.String[sScenesPath.Length() - 1] == ':' ? "" : "\\";
                m_FileName = sScenesPath + MYSTR(pSeparator) + sSceneName;
            }
            else
            {
                m_FileName = sSceneName;
            }

            sSceneName = m_FileName;
        }

        g_pSysInterface->m_sDefaultScene = sSceneName;
        if (g_pSysInterface->m_pSoundDll)
            g_pSysInterface->GetSoundDll()->PushScene(sSceneName);

        g_pSysFile->RemoveAllBigs();

        MYSTR sZipFile = CalcCacheFileName(m_FileName, "zip");
        ZLoader_Sequence_Player::Begin();
        InitAllocSequencePercent(nullptr, false);
        SetAllocSequencePercent(AS_ZIPLOAD, sSceneName, 0.0f);
        g_pSysFile->LoadWholeSceneZip(sZipFile);
        SetAllocSequencePercent(AS_ZIPLOAD, sSceneName, 1.0f);

        MYSTR sSupFile = CalcCacheFileName(m_FileName, "sup");
        const int lSupFileSize = g_pSysFile->GetSize(sSupFile, false);
        ZASSERT(lSupFileSize >= 0);
        if (lSupFileSize < 0)
            return;

        if (ISysMem::Exists())
            ISysMem::Instance().SetAllocDirection(ISysMem::AD_BACKWARD);
        void* pSupFileBuffer = ZUniMemory::Allocate(lSupFileSize);
        if (ISysMem::Exists())
            ISysMem::Instance().SetAllocDirection(ISysMem::AD_FORWARD);

        ZASSERT(g_pSysFile->Load(sSupFile, pSupFileBuffer, lSupFileSize, 0, false) == lSupFileSize);
        SetAllocSequencePercent(AS_INCLUDESCENE, sSceneName, 0.0f);
        AllocSequence(nullptr);
        g_pSysFile->RemoveAllBigs();
        m_pScene->LoadDoneNotify(nullptr);
        g_pSysInterface->UnlockRefs();
        ZLoader_Sequence_Player::End();
        g_pSysInterface->ResetTime();

        for (auto* pRender = g_pSysInterface->WindowFirst; pRender; pRender = pRender->Nxt)
            pRender->ColorFill();
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

    ZGROUP* ZEngineDataBase::CorrectEditorDestGroup(SCompiledGeom* pCompiledGeom, ZGROUP* pCurrentDestGroup)
    {
        // Editor-only dest-group correction; at runtime the group is returned unchanged.
        std::ignore = pCompiledGeom;
        return pCurrentDestGroup;
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
        // TODO: Finish me (PC sub_69D620)
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

        if (g_pDrawDebugTimer)
        {
            g_pDrawDebugTimer->StartFastTimer("UpdateList");
        }

        m_EventList.FrameUpdate();

        if (g_pDrawDebugTimer)
        {
            g_pDrawDebugTimer->EndFastTimer();
        }

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
        // PC 0x0045CB20.
        //
        // Builds the set of weapon prim ids that must survive the purge and then
        // frees every remaining weapon prim through the render DLL's prim control.
        // See SWeaponPrimEntry for the record layout (a TODO marker is left there
        // because the field semantics are still inferred from this function).
        const ZGameData* pGameData = g_pGameData;

        auto* pTotalWeaponPrims = reinterpret_cast<SWeaponPrimEntry*>(pGameData->GetTotalWeaponPrims());
        const uint32_t lNrTotalWeaponPrims = pGameData->GetTotalWeaponPrimsCount();
        const auto* pUsedWeaponPrims = reinterpret_cast<const SWeaponPrimEntry*>(pGameData->GetUsedWeaponPrims());
        const uint32_t lNrUsedWeaponPrims = pGameData->GetUsedWeaponPrimsCount();
        const auto* pBigWeaponPrims = reinterpret_cast<const SWeaponPrimEntry*>(pGameData->GetBigWeaponPrims());
        const uint32_t lNrBigWeaponPrims = pGameData->GetBigWeaponPrimsCount();

        // The weapon-hash keep-list only applies in the main menu scene (m00_main);
        // in every other scene the count is forced to 0 so it is ignored.
        int lWeaponHashesCount = 0;
        GetGlobalCom()->GetVal("WeaponHashesCount", &lWeaponHashesCount);
        if (_stricmp(g_pSysInterface->m_sDefaultScene.String, "m00_main") != 0)
            lWeaponHashesCount = 0;

        // Weapon hashes are stored as pairs of dwords.
        ZASSERT((lWeaponHashesCount & 1) == 0);

        uint32_t aWeaponHashes[128][2];
        uint32_t aWeaponPrimIds[128] = {};
        if (lWeaponHashesCount != 0)
        {
            const int lHashesDataLen = GetGlobalCom()->GetVal(
                reinterpret_cast<char*>(aWeaponHashes), "WeaponHashes", 0);
            ZASSERT(lHashesDataLen <= 0x400);
        }

        // Resolve each weapon-hash pair to the prim id of the matching total weapon prim.
        const int lNrWeaponHashes = lWeaponHashesCount / 2;
        if (lNrTotalWeaponPrims != 0)
        {
            for (int i = 0; i < lNrWeaponHashes; ++i)
            {
                for (uint32_t j = 0; j < lNrTotalWeaponPrims; ++j)
                {
                    if (aWeaponHashes[i][0] == pTotalWeaponPrims[j].m_lHash[0] &&
                        aWeaponHashes[i][1] == pTotalWeaponPrims[j].m_lHash[1])
                    {
                        aWeaponPrimIds[i] = pTotalWeaponPrims[j].m_lPrim;
                        break;
                    }
                }
            }
        }

        // When big-weapon purging is disabled, clear their prim ids so they are
        // not freed below.
        if (ZSysInterface::GetOption("NoHitmanBigWeaponsPurge", nullptr) && lNrBigWeaponPrims > 0)
        {
            for (uint32_t c = 0; c < lNrBigWeaponPrims; ++c)
            {
                for (uint32_t j = 0; j < lNrTotalWeaponPrims; ++j)
                {
                    if (pBigWeaponPrims[c].m_lPrim == pTotalWeaponPrims[j].m_lPrim)
                        pTotalWeaponPrims[j].m_lPrim = 0;
                }
            }
        }

        // Weapon prims currently in use must survive the purge.
        for (uint32_t c = 0; c < lNrUsedWeaponPrims; ++c)
        {
            for (uint32_t j = 0; j < lNrTotalWeaponPrims; ++j)
            {
                if (pUsedWeaponPrims[c].m_lPrim == pTotalWeaponPrims[j].m_lPrim)
                    pTotalWeaponPrims[j].m_lPrim = 0;
            }
        }

        // Keep the weapon-hash resolved prims as well.
        for (int i = 0; i < lNrWeaponHashes; ++i)
        {
            for (uint32_t j = 0; j < lNrTotalWeaponPrims; ++j)
            {
                if (aWeaponPrimIds[i] == pTotalWeaponPrims[j].m_lPrim)
                    pTotalWeaponPrims[j].m_lPrim = 0;
            }
        }

        // Drop duplicate prim ids so each surviving prim is only freed once.
        for (uint32_t a = 1; a < lNrTotalWeaponPrims; ++a)
        {
            for (uint32_t b = a; b < lNrTotalWeaponPrims; ++b)
            {
                if (pTotalWeaponPrims[a - 1].m_lPrim == pTotalWeaponPrims[b].m_lPrim)
                    pTotalWeaponPrims[b].m_lPrim = 0;
            }
        }

        // Free every remaining prim id (zeroed entries are no-ops).
        if (g_pRenderDll != nullptr && g_pRenderDll->m_pPrimControl != nullptr)
        {
            for (uint32_t j = 0; j < lNrTotalWeaponPrims; ++j)
                g_pRenderDll->m_pPrimControl->FreePrimData(pTotalWeaponPrims[j].m_lPrim);
        }
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

    // PS2 0x192978. Recalculates the root tree's center/size unless a min/max lock
    // is currently held by a caller.
    void ZEngineDataBase::CalcAllMinMax()
    {
        if (!m_lLockMinMax)
        {
            if (g_pEngineData->m_pRoot)
                g_pEngineData->m_pRoot->CalcCenSizeRecur();
        }
    }

    // PS2 0x1929FC. Walks every geom in the root tree and clears the per-geom
    // save/load control flag (ZCF bit 0x100000).
    void ZEngineDataBase::ClearSaveLoadFlags()
    {
        for (auto* pBaseGeom = ZROOT->BaseGeom(); pBaseGeom; ZROOT->RecurGetNext(&pBaseGeom))
            pBaseGeom->SetControlDirect(0, ZCBOUNDSDIRTY);
    }

    // PS2 0x19ADF8. Turns every deferred-creation geom collected during CreateGeoms
    // into a dynamic geom.
    void ZEngineDataBase::MakeDynamicGeomsDynamic(MakeDynArray* pMakeDynArray)
    {
        for (uint32_t i = 0; i < pMakeDynArray->Count(); ++i)
        {
            auto* pBaseGeom = ZBaseGeom::RefToPtr(pMakeDynArray->m_Array[i].rGeom);
            pBaseGeom->MakeDynamic(true);
        }
    }

    // PS2 0x19AEA4. Applies auto-room-assign to each deferred geom whose captured
    // control flags carried the auto-assign bit (0x4000).
    void ZEngineDataBase::MakeAutoAssignGeomsAutoAssign(MakeDynArray* pMakeDynArray)
    {
        for (uint32_t i = 0; i < pMakeDynArray->Count(); ++i)
        {
            const SMakeGeomDynamic& entry = pMakeDynArray->m_Array[i];
            if ((entry.lControl & ZCROOMASSIGN) != 0)
            {
                auto* pBaseGeom = ZBaseGeom::RefToPtr(entry.rGeom);
                pBaseGeom->SetAutoRoomAssign(true);
            }
        }
    }

    // PC 0x45AB50. Drives the root geom tree and the level event list through the
    // STATUS_Init then STATUS_Init2 phases, invoking the matching game-data hook
    // (ZGameData::Init / Init2) on each round.
    void ZEngineDataBase::Init()
    {
        PUSH_MEMORY_COLOR(0xA0A0A0);

        ZGEOM::m_PreferedStatus = ZGEOM::STATUS_Init;
        g_pEngineData->m_pRoot->DoInit();
        ZEventBase::m_DefaultStatus = ZEventBase::STATUS_Init;
        m_EventList.DoInit();
        if (g_pGameData)
            g_pGameData->Init();

        ZGEOM::m_PreferedStatus = ZGEOM::STATUS_Init2;
        g_pEngineData->m_pRoot->DoInit();
        ZEventBase::m_DefaultStatus = ZEventBase::STATUS_Init2;
        m_EventList.DoInit();
        if (g_pGameData)
            g_pGameData->Init2();
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
        PUSH_MEMORY_COLOR(0x606060u);
        MarkNonRunTime();

        const auto* pGms = reinterpret_cast<const SPackedGeomsHeader*>(pGeomsData);
        const char* pGmsBase = reinterpret_cast<const char*>(pGms);

        // The geom-type-count table lives at StartQuad[4] inside the GMS block:
        //   { u32 lNrGeomTypes; SGeomTypeCount aGeomTypes[lNrGeomTypes]; }
        // CountNrGeoms walks it to compute the base-geom and extra-geom pool sizes.
        uint32_t lNrBaseGeoms = 0;
        uint32_t lExtraGeomsSize = 0;
        const uint32_t lGeomTypeTableOffset = pGms->StartQuad[4];
        const uint32_t lNrGeomTypes = *reinterpret_cast<const uint32_t*>(pGmsBase + lGeomTypeTableOffset);
        auto* pGeomTypeCount = reinterpret_cast<SGeomTypeCount*>(const_cast<char*>(pGmsBase) + 4 + lGeomTypeTableOffset);
        CountNrGeoms(lNrBaseGeoms, lExtraGeomsSize, *pGeomTypeCount, lNrGeomTypes);

        // The packed entity table lives at StartQuad[0]:
        //   { u32 lNrEntities; SPackedGeomsTree aEntities[lNrEntities]; }
        const uint32_t lEntriesOffset = pGms->StartQuad[0];
        const uint32_t lNumberOfPackedGeoms = *reinterpret_cast<const uint32_t*>(pGmsBase + lEntriesOffset);

        if (g_pSysInterface->GetOption("PrintEngineInfo", nullptr))
        {
            ZINFO("Number of Packed Geoms: %d", lNumberOfPackedGeoms);
        }

        if (g_pSysInterface->GetOption("PrintEngineInfo", nullptr))
        {
            ZINFO("Highest used geomnumber: %d", pGms->m_iHighestGeomNr);
        }

        // Compute the four ZGeomBuffer pool sizes. The base/extra pools are
        // clamped to a 0x40000 floor; the event buffer base value is taken from
        // StartQuad[7] and tuned per default scene, matching the PC build.
        uint32_t lBaseGeomBufferSize = 0x70 * lNrBaseGeoms;
        if (lBaseGeomBufferSize < 0x40000)
            lBaseGeomBufferSize = 0x40000;

        if (lExtraGeomsSize < 0x40000)
            lExtraGeomsSize = 0x40000;

        const uint32_t lEventBase = pGms->StartQuad[7];
        uint32_t lEventBufferSize = lEventBase + 0x57800;
        if (strcmp(g_pSysInterface->m_sDefaultScene.String, "m10_main"))
            lEventBufferSize = lEventBase + 0x70800;
        if (strcmp(g_pSysInterface->m_sDefaultScene.String, "m09_main"))
            lEventBufferSize = lEventBase + 0x53000;
        if (lEventBufferSize < 0x40000)
            lEventBufferSize = 0x40000;

        if (!m_pGeomBuffer)
        {
            m_pGeomBuffer = ZUniMemory::New<ZGeomBuffer>(lBaseGeomBufferSize, lExtraGeomsSize, 0x14000u, lEventBufferSize);
        }

        // Allocate the ZROOT group if it does not exist yet.
        if (!m_pRoot)
        {
            m_pRoot = AllocRootGroup();
            m_pRoot->MakeDynamicContainer(true);
        }
        ZASSERT(m_pRoot);

        // ---- Preloader (PC 0x0045F2D0 / XBOX_KL1 0x82267E98) ----
        // Scratch arrays stay alive for the rest of the function: pGeomNrToRef
        // feeds the remap passes below, pBaseGeomArr feeds LoadProperties.
        constexpr uint32_t kClusterRowSize = 24;
        constexpr uint32_t kMaxClusterDepth = 128;

        const auto* pEntities = reinterpret_cast<const SPackedGeomsTree*>(pGmsBase + lEntriesOffset + 4);
        auto* pClusterDefs = reinterpret_cast<uint32_t*>(const_cast<char*>(pGmsBase) + 4 + pGms->StartQuad[5]);

        const uint32_t lNrGeomNrRefs = pGms->m_iHighestGeomNr + 1;
        ZREF* pGeomNrToRef = static_cast<ZREF*>(ZUniMemory::Allocate(sizeof(ZREF) * lNrGeomNrRefs));
        memset(pGeomNrToRef, 0, sizeof(ZREF) * lNrGeomNrRefs);
        auto** pBaseGeomArr = static_cast<ZBaseGeom**>(ZUniMemory::Allocate(sizeof(ZBaseGeom*) * lNumberOfPackedGeoms));
        auto** pClusterPool = static_cast<ZBaseGeom**>(ZUniMemory::Allocate(sizeof(ZBaseGeom*) * kClusterRowSize * kMaxClusterDepth));
        ZBaseGeom** pClusterSlots = pClusterPool;

        if (lNumberOfPackedGeoms)
        {
            bool bIsRootOfGroup = true;
            for (uint32_t i = 0; i < lNumberOfPackedGeoms; ++i)
            {
                if ((i & 0x1F) == 0)
                {
                    SetAllocSequencePercent(ALLOCSEQUENCESTATUS::AS_GEOMS, nullptr, (static_cast<float>(i) * 0.2f) / static_cast<float>(lNumberOfPackedGeoms));
                }

                const uint32_t lCompiledGeomOffset = pEntities[i].lCompiledGeomOffset;
                const uint32_t lDepth = lCompiledGeomOffset >> 25;
                const bool bGroupRoot = (lCompiledGeomOffset & 0x100000) != 0;
                auto* pCompiled = reinterpret_cast<SCompiledGeom*>(const_cast<char*>(pGmsBase) + ((lCompiledGeomOffset & 0xFFFFFF) << 2));

                // Walk up lDepth levels in the group hierarchy.
                if (lDepth)
                {
                    pClusterSlots -= lDepth * kClusterRowSize;
                    ZGROUP* pDestGroup = reinterpret_cast<ZGROUP*>(m_pRoot);
                    for (uint32_t d = 0; d < lDepth; ++d)
                    {
                        ZGROUP* pParent = pDestGroup->BaseGeom()->ParentGroup();
                        ZASSERT(pParent);
                        pDestGroup = pParent;
                    }
                    m_pRoot = reinterpret_cast<ZROOM*>(pDestGroup);
                }

                // First entity of a group: pre-allocate its typed lists from the
                // cluster-definition row and seed the working slot cursors.
                if (bIsRootOfGroup)
                {
                    uint32_t lTotalEntries = 0;
                    for (uint32_t j = 0; j < kClusterRowSize; ++j)
                        lTotalEntries += pClusterDefs[j];

                    if (lTotalEntries)
                    {
                        ZBaseGeom* pFirst = nullptr;
                        for (uint32_t k = 0; k < lTotalEntries; ++k)
                        {
                            auto* pBg = znew_placement<ZBaseGeom>(ZGeomBuffer::Instance().AllocBaseGeom());
                            if (k == 0)
                                pFirst = pBg;
                        }
                        ZASSERT(pFirst);

                        ZBaseGeom* pCursor = pFirst;
                        for (uint32_t j = 0; j < kClusterRowSize; ++j)
                        {
                            if (pClusterDefs[j])
                            {
                                pClusterSlots[j] = pCursor;
                                pCursor += pClusterDefs[j];
                            }
                            else
                            {
                                pClusterSlots[j] = nullptr;
                            }
                        }
                    }
                    else
                    {
                        for (uint32_t j = 0; j < kClusterRowSize; ++j)
                            pClusterSlots[j] = nullptr;
                    }
                    bIsRootOfGroup = false;
                }

                const char* pszName = pStaticBuffer + pCompiled->lOffsetName;
                const auto* pClassInfo = GetGeomClassInfo(pCompiled->lGeomType);
                const int32_t lListIndex = static_cast<int32_t>(GetBaseGeomListType(pClassInfo)) + 8 * pCompiled->cAssumedGeomDrawList;

                ZBaseGeom* pBaseGeom = pClusterSlots[lListIndex];
                pClusterSlots[lListIndex] = pBaseGeom + 1;
                ZASSERT(pBaseGeom);

                if (ForceExtraGeom() || pCompiled->CanExtraGeomBeZero())
                {
                    ZGeomBuffer::Instance().AllocGeom(pszName, pCompiled->lGeomType, pBaseGeom);

                    if (ZGEOM* pGeom = pBaseGeom->GetGeom())
                    {
                        ++pGeom->GetOldClassInfo()->m_lSceneInstanceCount;
                        pGeom->RegisterInstance(1);
                    }
                }

                ZGeomBuffer::Instance().m_bGeomCreationLock = true;

                const ZREF rGeom = ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom);
                prtCreatedGeoms->Add(rGeom);

                if (ZGEOM* pGeom = pBaseGeom->GetGeom())
                {
                    if (pCompiled->lExData)
                    {
                        const char* pExDataSrc = pStaticBuffer + pCompiled->lExData;
                        pGeom->CreateExData();

                        const char* pStatic = reinterpret_cast<const char*>(g_pEngineData->m_pStaticBuffer);
                        if (pStatic && pExDataSrc >= pStatic && pExDataSrc < pStatic + g_pEngineData->m_lStaticBufferLength)
                        {
                            pGeom->m_pExData->_ExtraInitData = reinterpret_cast<CHUNKFILE*>(const_cast<char*>(pExDataSrc));
                        }
                        else
                        {
                            const uint32_t lSize = *reinterpret_cast<const uint32_t*>(pExDataSrc + 4) & 0x3FFFFFFF;
                            void* pDst = ZUniMemory::Allocate(static_cast<int>(lSize));
                            memcpy(pDst, pExDataSrc, lSize);
                            pGeom->m_pExData->_ExtraInitData = reinterpret_cast<CHUNKFILE*>(pDst);
                        }
                    }
                }

                pBaseGeom->m_uListID = pEntities[i].lLightListID & 0xFFFFFF;

                m_pRoot = reinterpret_cast<ZROOM*>(CorrectEditorDestGroup(pCompiled, reinterpret_cast<ZGROUP*>(m_pRoot)));

                uint32_t lControl = pBaseGeom->Control() | (pCompiled->lGeomAndGroupCon & 0xFFFFF);
                if ((lControl & 0x8080) != 0)
                {
                    lControl |= 0x8080;
                }
                else
                {
                    pBaseGeom->m_uListID = 0;
                }

                if (pMakeDynArray && (lControl & ZCDYNAMIC) != 0)
                {
                    const ZREF rDyn = ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom);
                    const uint32_t lCount = pMakeDynArray->m_lNrEntries;
                    ZASSERT(lCount + 1 <= 0x3E8);
                    pMakeDynArray->m_Array[lCount].rGeom = rDyn;
                    pMakeDynArray->m_Array[lCount].lControl = lControl;
                    pMakeDynArray->m_lNrEntries = lCount + 1;
                    ZASSERT(pMakeDynArray->m_lNrEntries <= 0x3E8);
                    lControl &= 0xFFFBBFFF;
                }

                pBaseGeom->SetControl(lControl, ~lControl);
                m_pRoot->AttachGeom(pBaseGeom, false);
                pBaseGeom->SetControl(pBaseGeom->Control(), ~pBaseGeom->Control());

                if ((pCompiled->lGeomType & 0x100000) != 0) // may be a group -> next cluster-def row
                    pClusterDefs += kClusterRowSize;

                if (bGroupRoot)
                {
                    pClusterSlots += kClusterRowSize;
                    bIsRootOfGroup = true;
                    ZGEOM* pGroupGeom = pBaseGeom->GetGeom();
                    m_pRoot = reinterpret_cast<ZROOM*>(pGroupGeom);

                    // PS2-only safety checks (PS2 0x195894). PC 0x0045F2D0 has neither:
                    //  (1) the cluster-slot cursor must not overflow its scratch pool, and
                    //  (2) the group we descend into must really be a ZGROUP.
                    ZASSERT(pClusterSlots + kClusterRowSize <= pClusterPool + kClusterRowSize * kMaxClusterDepth);
                    ZASSERT(pGroupGeom && pGroupGeom->IsDerivedFrom<ZGROUP>());
                }

                if (ZGEOM* pGeom = pBaseGeom->GetGeom())
                {
                    if (auto* pGroup = geom_cast<ZGROUP>(pGeom))
                    {
                        const uint32_t lGrpCtl = pGroup->GroupControl();
                        const uint32_t lNewGrpCtl = (pCompiled->lGeomAndGroupCon & 0xFF000000) | lGrpCtl;
                        pGroup->SetGroupControl(lNewGrpCtl, ~lNewGrpCtl);
                    }
                }

                ZASSERT(pCompiled->m_iGeomNr);
                pGeomNrToRef[pCompiled->m_iGeomNr - 1] = ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom);
                pBaseGeomArr[i] = pBaseGeom;

                ZGeomBuffer::Instance().m_bGeomCreationLock = false;
            }
        }

        // ---- Load PRP properties (PC 0x0045F2D0) ----
        ZInputStream propertyStream(property_in_stream);
        ZPackedInput packedInput(&propertyStream);

        LoadZDefines(packedInput);

        if (m_pPathfinder4Data)
            m_pPathfinder4Data->RemapDoorRefs(pGeomNrToRef, lNrGeomNrRefs);

        if (g_pGameData)
            g_pGameData->RemapRefs(reinterpret_cast<uint32_t*>(pGeomNrToRef), lNrGeomNrRefs);

        ZMessageResolver::ResolveAll();

        g_iLoadPropertiesProgress = 0;
        g_iLoadPropertiesTotal = static_cast<int>(lNumberOfPackedGeoms);
        LoadProperties(lNumberOfPackedGeoms, pBaseGeomArr, packedInput);

        // Dev-tool repack marker: read-and-discard the "SimpleRepack" bool so the
        // property stream is consumed past it. Both PC 0x0045F2D0 and PS2 0x195894
        // read it into a throwaway local; the runtime g_pSysInterface->m_bSimpleRepack
        // flag is a separate config value set elsewhere. Guarded by a non-empty scene.
        if (lNumberOfPackedGeoms)
        {
            bool bSimpleRepack = false;
            packedInput.Exchange("SimpleRepack", bSimpleRepack);
            std::ignore = bSimpleRepack;
        }

        packedInput.End();

        // ---- Resolve references in ZROOMs (PC 0x0045F2D0) ----
        for (auto* pBaseGeom = m_pRoot->BaseGeom(); pBaseGeom; m_pRoot->RecurGetNext(&pBaseGeom))
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            const bool bIsRoom = pGeom
                ? ((pGeom->GetObjectId() & ZROOM::m_Mask) == ZROOM::m_Id)
                : pBaseGeom->IsDerivedFrom<ZROOM>();
            if (bIsRoom && pGeom)
                reinterpret_cast<ZROOM*>(pGeom)->RemapRefs(pGeomNrToRef, lNrGeomNrRefs);
        }

        // ---- Light list (PC 0x0045F2D0) ----
        if (!g_pSysInterface->m_bDisableLight)
        {
            const uint32_t lLightOffset = pGms->StartQuad[6];
            if (lLightOffset)
            {
                const uint32_t lSize = *reinterpret_cast<const uint32_t*>(pGmsBase + lLightOffset);
                PUSH_MEMORY_COLOR(0x20FF7Fu);
                void* pLightBuf = ZUniMemory::Allocate(static_cast<int>(lSize));
                memcpy(pLightBuf, pGmsBase + 4 + lLightOffset, lSize);
                m_pListUser = ZUniMemory::New<CListUser>(pLightBuf);
                ZASSERT(m_pListUser);
                m_pListUser->ConvertOffsetsToRefs(reinterpret_cast<const uint32_t*>(pGeomNrToRef));
            }
        }

        m_pGeomBuffer->InitResourceGeoms(const_cast<SPackedGeomsHeader*>(pGms));
        if (BS_Runtime::ZMaterialDescriptionDB::m_Instance)
            BS_Runtime::ZMaterialDescriptionDB::m_Instance->RemapGeoms(reinterpret_cast<uint32_t*>(pGeomNrToRef));

        // All remap/property passes have consumed the preloader scratch buffers.
        ZUniMemory::Free(pClusterPool);
        ZUniMemory::Free(pBaseGeomArr);
        ZUniMemory::Free(pGeomNrToRef);
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
