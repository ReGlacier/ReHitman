#include <Glacier/Render/Entry/ZRenderEntryCameraD3D.h>
#include <Glacier/Render/Prim/SPrimLightSpot.h>
#include <Glacier/Render/Prim/SPrimLightSpotSquare.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/Prim/SPrimLight.h>
#include <Glacier/Render/Prim/ELightType.h>
#include <Glacier/Render/Cmd/ZRenderCommands.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Entry/SRenderEntryNotifyInfo.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/Camera/ZCameraSpace.h>
#include <Glacier/Render/ZVolumeList.h>
#include <Glacier/Render/Draw/ZDrawBufferSimple.h>
#include <Glacier/Render/ZRenderWintelD3D.h>
#include <Glacier/Render/ZViewSpace.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Render/ZShadowListBuild.h>
#include <Glacier/Render/ZShadowListUser.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryLists.h>
#include <Glacier/Render/Entry/ZRenderEntryBones.h>
#include <Glacier/Render/View/ZRenderView.h>
#include <Glacier/Geom/ZENVIRONMENT.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        struct ShadowCandidate
        {
            const ZBaseGeom* pLight;
            const ZBaseGeom* pCaster;
            const ZBaseGeom** ppReceivers;
            uint32_t lNumReceivers;
            float fPriority;
            float fDistance;
        };

        struct ShadowProjectionPayload
        {
            uint8_t lType;
            uint8_t pad[3];
            uint32_t lPrim;
            uint8_t data[0x88];
        };
        RE_VERIFY_SIZE(ShadowProjectionPayload, 0x90);

        struct ShadowReceiverPayload
        {
            uint8_t data[0x74];
        };
        RE_VERIFY_SIZE(ShadowReceiverPayload, 0x74);

        struct ShadowSetupPayload
        {
            float mProjection[12];
            uint32_t lColor;
        };
        RE_VERIFY_SIZE(ShadowSetupPayload, 0x34);

        ZCmdList::ZCmd* BeginShadowCommand(ZCmdList* pCmdList, ZRenderView* pRenderView, ZCmdList::CMD eType)
        {
            auto* pCmd = pCmdList->Current();
            pCmd->m_lType = eType;
            pCmd->m_pRenderEntryGeom = nullptr;
            pCmd->m_pCmdList = pCmdList;
            pCmd->m_pRenderView = pRenderView;
            pCmd->m_lLayer = 0;
            pCmd->m_lNrObjects = 0;
            pCmdList->NextCommand();
            return pCmd;
        }

        template <typename T>
        void AddShadowPayload(ZCmdList::ZCmd* pCmd, const T& payload)
        {
            pCmd->AddData(const_cast<T*>(&payload), sizeof(payload));
        }

        ZRenderEntry* EntryForBaseGeom(ZRenderDraw* pDraw, const ZBaseGeom* pBaseGeom)
        {
            return pBaseGeom && pBaseGeom->m_lDrawId
                ? pDraw->m_apRenderEntryLookup[pBaseGeom->m_lDrawId & 0x7FFFu]
                : nullptr;
        }

        ZCmdList::ZCmd* BeginCommand(ZCmdList* pCmdList, ZRenderView* pView, ZCmdList::CMD eType)
        {
            auto* pCmd = pCmdList->Current();
            pCmd->m_lType = eType;
            pCmd->m_pRenderEntryGeom = nullptr;
            pCmd->m_pCmdList = pCmdList;
            pCmd->m_pRenderView = pView;
            pCmd->m_lLayer = 0;
            pCmd->m_lNrObjects = 0;
            pCmdList->NextCommand();
            return pCmd;
        }

    }

    ZRenderEntryCameraD3D::ZRenderEntryCameraD3D(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryCamera(sInfo)
    {
        int32_t lEnableDotShadows = 0;
        g_pEngineData->GetSceneCom()->GetVal("PC_XBOX_EnableDotFadeDropShadows", &lEnableDotShadows);
        g_PC_XBOX_EnableDotFadeDropShadows = lEnableDotShadows != 0;
    }

    ZRenderEntryCameraD3D::~ZRenderEntryCameraD3D() = default;

    void ZRenderEntryCameraD3D::GetVisible(ZCmdList* pCmdList, ZRenderEntryGeom* pGeomEntry, ZViewSpace* pViewSpace, ZRenderView* pView, ZRenderEntryLists* pEntryList)
    {
        auto* pCamera = static_cast<ZCAMERA*>(pGeomEntry->GetBaseGeom()->GetGeom());
        auto* pRender = pView->m_pRender;
        auto* pDraw = static_cast<ZRenderDraw*>(IDraw::Instance());

        ZCameraSpace cameraSpace;
        cameraSpace = pCamera;
        pViewSpace->SetClipPlanesFromCameraSpace(&cameraSpace);

        const float fLODScale = std::tan(cameraSpace.CalcFieldOfViewX() * 0.5f) /
            std::tan(0.5881760120391846f) * 0.01f;
        const ZVector3 vObserver = m_ObjectToWorldMatrix.p0;
        const uint32_t* pViewport = pView->Viewport();
        ZVector4 cameraViewport;
        pCamera->GetViewport(&cameraViewport);
        const uint32_t lViewportWidth = pViewport[2] - pViewport[0];
        const uint32_t lViewportHeight = pViewport[3] - pViewport[1];

        static_cast<ZRenderWintelD3D*>(pRender)->CreateFrustumFromCameraSpace(
            reinterpret_cast<ZMat4x4*>(&m_matProjection), &cameraSpace, pViewport,
            cameraSpace.GetFar() == 0.0f, false);

        ZVolumeList volumeList;
        pViewSpace->GetVisibleVolumesIncludingBackdrop(&volumeList, this, (pCamera->CameraCon & 0x2000) == 0);
        ZRenderEntryLists lists;
        pEntryList->m_pViewList = &lists;
        ZRenderEntry* apRenderEntries[2048] {};
        const uint32_t lNumRenderEntries = pDraw->CreateRenderEntries(apRenderEntries, 2048,
            &volumeList, pEntryList, this, const_cast<float*>(&vObserver.x), fLODScale);

        auto* pDrawUpdate = lists.GetList(ZRenderEntryLists::LT_DRAWUPDATE);
        ZDrawBufferSimple drawBuffer(apRenderEntries + lNumRenderEntries, lViewportWidth, lViewportHeight);
        for (uint32_t i = 0; i < pDrawUpdate->Count(); ++i)
        {
            auto* pEntry = *pDrawUpdate->Get(i);
            pEntry->GetBaseGeom()->GetGeom()->DrawBufferViewUpdate(&drawBuffer, &cameraSpace);
        }

        ZRenderEntry* apEnvironment[2048] {};
        uint32_t lNumEnvironment = 0;
        auto* pEnvironment = lists.GetList(ZRenderEntryLists::LT_ENVIRONMENT);
        for (uint32_t i = 0; i < pEnvironment->Count(); ++i)
            apEnvironment[lNumEnvironment++] = *pEnvironment->Get(i);
        if (lNumEnvironment)
            CmdDrawEntries(pCmdList, pView, const_cast<const ZRenderEntry**>(apEnvironment), lNumEnvironment,
                vObserver, fLODScale, 13, 1, 3);

        CmdDrawEntries(pCmdList, pView, const_cast<const ZRenderEntry**>(apRenderEntries), lNumRenderEntries,
            vObserver, fLODScale, 6, 1, 3);
        CmdDrawEntries(pCmdList, pView, const_cast<const ZRenderEntry**>(apRenderEntries), lNumRenderEntries,
            vObserver, fLODScale, 7, 1, 3);
        CmdDrawEntries(pCmdList, pView, const_cast<const ZRenderEntry**>(apRenderEntries), lNumRenderEntries,
            vObserver, fLODScale, 8, 1, 3);

        BuildDrawChainDropShadows(pGeomEntry, pView, pCmdList, pEntryList, &m_lNumShadows);

        if (!g_bDisablePostEffects && pView->GetPostFilter())
        {
            BeginCommand(pCmdList, pView, ZCmdList::CMD_APPLY_POSTFILTER);
        }

        pEntryList->m_pViewList = nullptr;
    }

    ZRenderEntryCameraD3D* ZRenderEntryCameraD3D::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        if (!sInfo.m_pBaseGeom)
            return nullptr;

        const auto* pGeom = sInfo.m_pBaseGeom->GetGeom();
        const bool bCamera = pGeom
            ? pGeom->IsDerivedFrom<ZCAMERA>()
            : sInfo.m_pBaseGeom->IsDerivedFromStdObj(ZCAMERA::m_Id);
        if (!bCamera)
            return nullptr;

        return ZUniMemory::New<ZRenderEntryCameraD3D>(sInfo);
    }

    void ZRenderEntryCameraD3D::BuildDrawChainDropShadows(ZRenderEntryGeom* pParentRenderEntryGeom, ZRenderView* pRenderView, ZCmdList* pCmdList, ZRenderEntryLists* pGeomList, uint32_t* plMaxProjections)
    {
        auto* pShared = ZSharedResourcesD3D::g_pInstance;
        auto* pListUser = g_pEngineData->m_pListUser;
        if (!pShared || !pListUser || !g_pSysInterface->m_lShadowDetail || !pShared->m_lShadowMapPassesNr)
            return;
        const char* pName = pParentRenderEntryGeom->GetBaseGeom()->Name();
        if (pName && strcmp(pName, "ZWindowsCamera") == 0)
            return;

        auto* pLists = pGeomList->m_pViewList;
        if (!pLists)
            return;
        auto* pDraw = IDraw::Instance<ZRenderDraw>();
         const ZVector3 vObserver = m_ObjectToWorldMatrix.p0;
        const float fLODScale = m_matProjection._11;

        uint8_t aCompiled[4096] {};
        ZShadowListBuild builder;
        builder.Init(aCompiled, sizeof(aCompiled));
        auto* pLights = pLists->GetList(ZRenderEntryLists::LT_LIGHT);
         for (uint32_t i = 0; i < pLights->Count(); ++i)
         {
            auto* pLightEntry = *pLights->Get(i);
            auto* pLight = pLightEntry ? pLightEntry->GetBaseGeom() : nullptr;
             if (pLight && !pLight->IsDerivedFrom<ZENVIRONMENT>())
             {
                 // The PC build also rejects light primitive bone-declaration
                 // chunks here. SPrimInfo is not represented in the current
                 // headers, so do not invent its payload layout.
                 builder.ProcessLight(pListUser, pLight);
             }
        }
        builder.ProcessEnvironment(pLists->GetList(ZRenderEntryLists::LT_BONES));
        builder.Compile();

        ZRenderEntry* apRenderEntries[2048] {};
        uint32_t lNumEntries = 0;
        auto* pStatic = pLists->GetList(ZRenderEntryLists::LT_STATICSHADOW);
        for (uint32_t i = 0; i < pStatic->Count(); ++i)
        {
            auto* pEntry = *pStatic->Get(i);
            if (pEntry && (pEntry->m_lControl & ZRenderEntry::RE_STATIC_SHADOW_SUB) == 0)
                apRenderEntries[lNumEntries++] = pEntry;
        }
        CmdDrawEntries(pCmdList, pRenderView, const_cast<const ZRenderEntry**>(apRenderEntries), lNumEntries,
            vObserver, fLODScale, 4, 1, 7);

        ShadowCandidate aCandidates[16] {};
        uint32_t lNumCandidates = 0;
        ZShadowListUser user;
        user.Init(aCompiled);
        for (int s = 0; s < user.GetNumSets(); ++s)
        {
            SetHeader* pSet = user.GetSetHeader();
            for (int c = 0; c < pSet->lNumCasters; ++c)
            {
                CasterHeader* pCaster = user.GetCasterHeader();
                const ZBaseGeom** ppReceivers = user.GetReceiverBase(pCaster->lNumReceivers);
                ZRenderEntry* pCasterEntry = EntryForBaseGeom(pDraw, pCaster->pCaster);
                if (!pCasterEntry)
                    continue;
                auto* pBonesEntry = static_cast<ZRenderEntryBones*>(pCasterEntry);
                const float fDistance = pBonesEntry->m_fMinDistanceToObservers;
                if (fDistance > 2000.0f)
                    continue;
                const float fFade = clamp((2000.0f - fDistance) * 0.21333334f, 0.0f, 64.0f);
                const float fPriority = CalculateShadowIntensity(pSet->pLight, pCaster->pCaster) *
                    (1.0f - fDistance * 0.00015f) * fFade;
                if (fPriority <= 0.0f)
                    continue;
                uint32_t lInsert = lNumCandidates;
                while (lInsert && fPriority > aCandidates[lInsert - 1].fPriority)
                    --lInsert;
                if (lInsert >= 16)
                    continue;
                const uint32_t lEnd = (std::min)(lNumCandidates, 15u);
                for (uint32_t lMove = lEnd; lMove > lInsert; --lMove)
                    aCandidates[lMove] = aCandidates[lMove - 1];
                aCandidates[lInsert] = { pSet->pLight, pCaster->pCaster, ppReceivers,
                    static_cast<uint32_t>(pCaster->lNumReceivers), fPriority, fDistance };
                lNumCandidates = (std::min)(lNumCandidates + 1, 16u);
            }
        }

        BeginShadowCommand(pCmdList, nullptr, static_cast<ZCmdList::CMD>(36));
        const uint32_t lBatchSize = (std::min)(4u, static_cast<uint32_t>(pShared->m_lShadowMapPassesNr));
        for (uint32_t first = 0; first < lNumCandidates; first += lBatchSize)
        {
            const uint32_t count = (std::min)(lBatchSize, lNumCandidates - first);
            bool abValid[4] {};
            for (uint32_t i = 0; i < count; ++i)
            {
                const ShadowCandidate& candidate = aCandidates[lNumCandidates - first - i - 1];
                ZRenderEntry* pCaster = EntryForBaseGeom(pDraw, candidate.pCaster);
                if (!pCaster)
                    continue;
                abValid[i] = true;
                 ShadowProjectionPayload projection {};
                projection.lType = 1;
                projection.lPrim = candidate.pLight->m_lPrim;
                AddShadowPayload(BeginShadowCommand(pCmdList, pRenderView, static_cast<ZCmdList::CMD>(3)), projection);
                ShadowReceiverPayload receiver {};
                AddShadowPayload(BeginShadowCommand(pCmdList, pRenderView, static_cast<ZCmdList::CMD>(6)), receiver);
                const ZRenderEntry* pCasterEntries[] = { pCaster };
                CmdDrawEntries(pCmdList, pRenderView, pCasterEntries, 1, vObserver, fLODScale, 2, 0x19, 7);
            }
            BeginShadowCommand(pCmdList, pRenderView, ZCmdList::CMD_SHADOW_CASTERS_START_2);
            for (uint32_t i = 0; i < count; ++i)
            {
                if (!abValid[i])
                    continue;
                const ShadowCandidate& candidate = aCandidates[lNumCandidates - first - i - 1];
                lNumEntries = 0;
                for (uint32_t r = 0; r < candidate.lNumReceivers && lNumEntries < 2048; ++r)
                    if (auto* pEntry = EntryForBaseGeom(pDraw, candidate.ppReceivers[r]))
                        apRenderEntries[lNumEntries++] = pEntry;
                ShadowReceiverPayload receiver {};
                AddShadowPayload(BeginShadowCommand(pCmdList, pRenderView, static_cast<ZCmdList::CMD>(6)), receiver);
                CmdDrawEntries(pCmdList, pRenderView, const_cast<const ZRenderEntry**>(apRenderEntries), lNumEntries,
                    vObserver, fLODScale, 5, 1, 7);
            }
            BeginShadowCommand(pCmdList, pRenderView, ZCmdList::CMD_SHADOW_RECEIVERS_START);

            for (uint32_t i = 0; i < count; ++i)
            {
                if (!abValid[i])
                    continue;
                const ShadowCandidate& candidate = aCandidates[lNumCandidates - first - i - 1];
                ShadowReceiverPayload receiver {};
                AddShadowPayload(BeginShadowCommand(pCmdList, pRenderView, static_cast<ZCmdList::CMD>(6)), receiver);
                ShadowSetupPayload setup {};
                memcpy(setup.mProjection, candidate.pCaster->m_mMat, sizeof(candidate.pCaster->m_mMat));
                const uint32_t alpha = static_cast<uint32_t>((std::min)(candidate.fPriority * 4.24f, 255.0f));
                setup.lColor = (alpha << 24) | 0x00FFFFFFu;
                AddShadowPayload(BeginShadowCommand(pCmdList, pRenderView, ZCmdList::CMD_SHADOW_SETUP), setup);
                BeginShadowCommand(pCmdList, pRenderView, ZCmdList::CMD_CLEAR_AND_BEGIN_ALPHABLEND_MAX);
                lNumEntries = 0;
                for (uint32_t r = 0; r < candidate.lNumReceivers && lNumEntries < 2048; ++r)
                    if (auto* pEntry = EntryForBaseGeom(pDraw, candidate.ppReceivers[r]))
                        apRenderEntries[lNumEntries++] = pEntry;
                CmdDrawEntries(pCmdList, pRenderView, const_cast<const ZRenderEntry**>(apRenderEntries), lNumEntries,
                    vObserver, fLODScale, 3, 1, 7);
                BeginShadowCommand(pCmdList, pRenderView, ZCmdList::CMD_BEGIN_ALPHABLEND_MAX);
            }
        }

        BeginShadowCommand(pCmdList, pRenderView, ZCmdList::CMD_CLEAR_AND_BEGIN_ALPHABLEND_MAX);
        lNumEntries = 0;
        for (uint32_t i = 0; i < pStatic->Count(); ++i)
        {
            auto* pEntry = *pStatic->Get(i);
            if (pEntry && (pEntry->m_lControl & ZRenderEntry::RE_STATIC_SHADOW_SUB) != 0)
                apRenderEntries[lNumEntries++] = pEntry;
        }
        CmdDrawEntries(pCmdList, pRenderView, const_cast<const ZRenderEntry**>(apRenderEntries), lNumEntries,
            vObserver, fLODScale, 0x11, 1, 7);

        auto* pBones = pLists->GetList(ZRenderEntryLists::LT_BONES);
        uint32_t lMaxProjections = 0;
        for (uint32_t i = 0; i < pBones->Count(); ++i)
        {
            auto* pEntry = *pBones->Get(i);
            if (pEntry)
                lMaxProjections = (std::max)(lMaxProjections, pEntry->m_unk14);
        }
        if (plMaxProjections)
            *plMaxProjections = lMaxProjections;
        BuildDrawChainStaticShadowTextureProjections(pCmdList, lMaxProjections,
            *reinterpret_cast<ZStackArray<1024, ZRenderEntryGeom*>*>(pBones), pRenderView,
            vObserver, fLODScale, 0, 0xC);
        BeginShadowCommand(pCmdList, pRenderView, ZCmdList::CMD_BEGIN_ALPHABLEND_MAX);
        const uint32_t* pViewport = pRenderView->Viewport();
        uint32_t viewport[4] { pViewport[0], pViewport[1], pViewport[2] - pViewport[0], pViewport[3] - pViewport[1] };
        auto* pViewportCmd = BeginShadowCommand(pCmdList, pRenderView, ZCmdList::CMD_END_ALPHABLEND_MAX);
        pViewportCmd->AddData(viewport, sizeof(viewport));
    }

    void ZRenderEntryCameraD3D::BuildDrawChainStaticShadowTextureProjections(
        ZCmdList* pCmdList,
        uint32_t lMaxProjections,
        ZStackArray<1024u, ZRenderEntryGeom*>& LnkList,
        ZRenderView* pRenderView,
        const ZVector3& vObserver,
        float fLODScale,
        uint32_t lType,
        uint32_t lLayer)
    {
        auto* const pRenderDraw = static_cast<ZRenderDraw*>(IDraw::Instance());

        for (uint32_t lProjection = 0; lProjection < lMaxProjections; ++lProjection)
        {
            ZRenderEntry* apRenderEntries[2048] {};
            uint32_t lNumRenderEntries = 0;

            for (uint32_t i = 0; i < LnkList.Count(); ++i)
            {
                ZRenderEntryGeom* pEntry = *LnkList.Get(i);
                if (!pEntry || pEntry->m_unk14 <= lProjection)
                    continue;

                auto* pInstance = pEntry->m_RenderEntryInstanceTable[lProjection];
                if (!pInstance || !pInstance->pRenderObjectInstance
                    || (pInstance->pRenderObjectInstance->m_lFlags & 0x20000000u) != (lType << 29))
                    continue;

                ZASSERT(lNumRenderEntries < 2048);
                apRenderEntries[lNumRenderEntries++] = pEntry;

                ZStackArray<1024, ZRenderEntry::ZAttachedBaseGeom> attachedGeoms;
                pEntry->GetAttachedBaseGeoms(&attachedGeoms);
                for (uint32_t j = 0; j < attachedGeoms.Count(); ++j)
                {
                    const uint16_t lDrawId = attachedGeoms.Get(j)->m_pBaseGeom->m_lDrawId;
                    if (!lDrawId)
                        continue;

                    ZRenderEntry* pAttachedEntry = pRenderDraw->m_apRenderEntryLookup[lDrawId & 0x7FFFu];
                    if (pAttachedEntry)
                    {
                        ZASSERT(lNumRenderEntries < 2048);
                        apRenderEntries[lNumRenderEntries++] = pAttachedEntry;
                    }
                }
            }

            if (!lNumRenderEntries)
                continue;

            ZCmdList::ZCmd* pCmd = pCmdList->Current();
            pCmd->m_lType = static_cast<ZCmdList::CMD>(40);
            pCmd->m_pRenderEntryGeom = nullptr;
            pCmd->m_pCmdList = pCmdList;
            pCmd->m_pRenderView = pRenderView;
            pCmd->m_lLayer = 18;
            pCmd->m_lNrObjects = 0;
            pCmdList->NextCommand();

            *reinterpret_cast<uint32_t*>(pCmd->AddData(sizeof(uint32_t))) = lProjection;

            SRenderEntryNotifyInfo sNotifyInfo {};
            sNotifyInfo.fLODScale = fLODScale;
            sNotifyInfo.vObserver[0] = vObserver.x;
            sNotifyInfo.vObserver[1] = vObserver.y;
            sNotifyInfo.vObserver[2] = vObserver.z;

            for (uint32_t i = 0; i < lNumRenderEntries; ++i)
            {
                auto* pEntry = apRenderEntries[i];
                pEntry->CalcLODMask(&sNotifyInfo);
                pEntry->AddToDrawChain(pCmd, 0, 1u << lLayer, 1, 7, false);
            }
        }
    }

    float ZRenderEntryCameraD3D::CalculateShadowIntensity(const ZBaseGeom* pLight, const ZBaseGeom* pCaster)
    {
        ZVector3 vCaster;
        ZVector3 vVect(0.0f, 1.0f, 0.0f);

        pCaster->GetCen(vCaster);
        pCaster->GetRootPoint(vCaster);
        const_cast<ZBaseGeom*>(pLight)->GetLocalPointVect(vCaster, vVect);

        const auto* const pLightPrim = ZPrimHandle{ pLight->m_lPrim }.Get<SPrimLightSpot>();

        if (pLightPrim->lLightType == LTENVIRONMENT || pLightPrim->lLightType == LTDIRECTIONAL)
        {
            return 1.0f;
        }
        else if (pLightPrim->lLightType > 2u)
        {
            return 0.0f;
        }

        const float fDist = 1.0f / (pLightPrim->fFarRange - pLightPrim->fNearRange);
        const float fInvDist = -fDist;
        const float fIntensity = pLightPrim->fFarRange * fDist;

        ZVector3 vToLight;
        vToLight = vVect;
        const_cast<ZBaseGeom*>(pLight)->GetLocalPointVect(vCaster, vToLight);
        const float fRadialBottleneck = (std::max)(0.0f, vnorm(vToLight.Get(), vCaster.Get()) - pCaster->m_fRadius);
        const float fAttenuation = clamp(fIntensity + fRadialBottleneck * fInvDist, 0.0f, 1.0f);

        vneg(vToLight.Get());

        float fShadowIntensity;
        if (g_PC_XBOX_EnableDotFadeDropShadows)
        {
            const float fDotFade = vdot(vToLight.Get(), vVect.Get()) * fAttenuation;
            fShadowIntensity = (std::max)(0.0f, fDotFade);
        }
        else
        {
            fShadowIntensity = fAttenuation;
        }

        const uint8_t lLightType = pLightPrim->lLightType;
        if (lLightType != LTSPOT && lLightType != LTSPOTSQUARE)
        {
            return fShadowIntensity;
        }

        if (vCaster.z <= 0.0f)
        {
            return 0.0f;
        }

        float fConeFactor;
        if (lLightType == LTSPOTSQUARE)
        {
            const auto* const pSquare = static_cast<const SPrimLightSpotSquare*>(pLightPrim);
            const float fTanFallOff = std::tan(pSquare->fFallOff);
            const float fSquareSize = std::sqrt(pSquare->fAspect) * fTanFallOff;
            const float fHotSpotRange = 1.0f - std::tan(pSquare->fHotSpot) / fTanFallOff;
            const float fInvDepth = 1.0f / vCaster.z;
            const float fOffset = -1.0f / fHotSpotRange;
            float fX = std::fabs((1.0f / (fHotSpotRange * fSquareSize)) * vCaster.x * fInvDepth) + fOffset;
            float fY = std::fabs((1.0f / (fSquareSize / pSquare->fAspect * fHotSpotRange)) * vCaster.y * fInvDepth) + fOffset;
            fX = clamp(fX, -1.0f, 0.0f);
            fY = clamp(fY, -1.0f, 0.0f);
            fConeFactor = fY * fX;
        }
        else
        {
            const float fInvRange = 1.0f / (pLightPrim->fCosHotSpot - pLightPrim->fCosFallOff);
            const float fCone = fInvRange * (-vToLight.z - pLightPrim->fCosFallOff);
            fConeFactor = clamp(fCone, 0.0f, 1.0f);
        }

        return fShadowIntensity * fConeFactor * fConeFactor * (3.0f - 2.0f * fConeFactor);
    }
}
