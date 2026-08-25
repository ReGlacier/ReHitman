#include <Glacier/Render/Cmd/ZRenderCommands.h>
#include <Glacier/Render/Cmd/ZCmdList.h>


namespace Glacier
{
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
        pCmd->m_lLayer = ZCmdList::CMD_OBJECT_DRAW;
        pCmd->m_pRenderEntryGeom = nullptr;
        pCmd->m_pCmdList = pCmdList;
        pCmd->m_pRenderView = pRenderView;
        pCmd->m_lLayer = lLayer;
        pCmd->m_lNrObjects = 0;

        // TODO: Finish me
        // PC: 004B2410
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
