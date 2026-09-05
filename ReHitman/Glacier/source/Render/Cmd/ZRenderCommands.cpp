#include <Glacier/Render/Cmd/ZRenderCommands.h>
#include <Glacier/Render/Cmd/ZCmdList.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Entry/SRenderEntryNotifyInfo.h>
#include <Glacier/Render/Entry/ZRenderEntry.h>


namespace Glacier
{
    namespace
    {
        int CmpRenderObjectInstances(const void* a, const void* b)
        {
            const ZRenderObjectInstance* pA = *(const ZRenderObjectInstance**)(a);
            const ZRenderObjectInstance* pB = *(const ZRenderObjectInstance**)(b);

            if (pA->m_lSortValue == pB->m_lSortValue)
            {
                return pA - pB;
            }

            return pA->m_lSortValue - pB->m_lSortValue;
        }

        void SortRenderObjectInstances(ZRenderObjectInstance** ppInstances, uint32_t lCount)
        {
            qsort((void*)ppInstances, lCount, 4, CmpRenderObjectInstances);
        }
    }

    void CmdSetViewport(ZCmdList* pCmdList, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
    {
        ZCmdList::ZCmd* pCmd = pCmdList->Current();
        pCmd->m_lLayer = ZCmdList::CMD_VIEWPORT_SETUP;
        pCmd->m_pRenderEntryGeom = nullptr;
        pCmd->m_pCmdList = pCmdList;
        pCmd->m_pRenderView = nullptr;
        pCmd->m_lLayer = 0;
        pCmd->m_lNrObjects = 0;

        pCmdList->NextCommand();

        uint32_t* pViewport = reinterpret_cast<uint32_t*>(pCmd->AddData(sizeof(uint32_t) * 4));
        pViewport[0] = x;
        pViewport[1] = y;
        pViewport[2] = w;
        pViewport[3] = h;
    }

    void CmdDrawEntries(
        ZCmdList* pCmdList,
        ZRenderView* pRenderView,
        const ZRenderEntry** apRenderEntries,
        uint32_t lNumRenderEntries,
        const ZVector3& vObserver,
        float fLODScale,
        uint32_t lLayer,
        uint32_t lDrawDestination,
        uint32_t lTransparencyMask
    )
    {
        ZCmdList::ZCmd* pCmd = pCmdList->Current();
        pCmd->m_lType = static_cast<ZCmdList::CMD>(0x11); // PC CmdDrawEntries writes 17 for object draws.
        pCmd->m_pRenderEntryGeom = nullptr;
        pCmd->m_pCmdList = pCmdList;
        pCmd->m_pRenderView = pRenderView;
        pCmd->m_lNrObjects = 0;
        pCmdList->NextCommand();

        pCmd->m_lLayer = lLayer;

        SRenderEntryNotifyInfo sInfo {};
        sInfo.fLODScale = fLODScale;
        sInfo.vObserver[0] = vObserver.x;
        sInfo.vObserver[1] = vObserver.y;
        sInfo.vObserver[2] = vObserver.z;
        sInfo.bMirror = false;
        sInfo.bFirstPersonCamera = false;
        sInfo.lDrawDestinationOverride = 0;

        for (int i = 0; i < lNumRenderEntries; ++i)
        {
            auto* pEntry = const_cast<ZRenderEntry*>(apRenderEntries[i]);

            pEntry->CalcLODMask(&sInfo);
            pEntry->Notify(&sInfo);
            pEntry->AddToDrawChain(pCmd, &sInfo, 1 << lLayer, lDrawDestination, lTransparencyMask, false);
        }

        SortRenderObjectInstances(reinterpret_cast<ZRenderObjectInstance**>(pCmd + 1), pCmd->m_lNrObjects);
    }

    void CmdScissorSetup(ZCmdList* pCmdList, ZRenderView* pRenderView, const ZVector4& vScissor, bool bUnk)
    {
        ZCmdList::ZCmd* pCmd = pCmdList->Current();
        pCmd->m_lLayer = ZCmdList::CMD_SCISSOR_SETUP;
        pCmd->m_pRenderEntryGeom = nullptr;
        pCmd->m_pCmdList = pCmdList;
        pCmd->m_pRenderView = pRenderView;
        pCmd->m_lLayer = 0x30000000u; // ???
        pCmd->m_lNrObjects = 0;

        pCmdList->NextCommand();

        // Based on iOS: in IOS used 128bit single variable and used weird hack to add data (inlined, I guess?)
        auto* pData = reinterpret_cast<ZVector4*>(pCmd->AddData(sizeof(ZVector4)));
        pData->x = vScissor.x;
        pData->y = vScissor.y;
        pData->z = vScissor.z;
        pData->w = vScissor.w;
    }
}
