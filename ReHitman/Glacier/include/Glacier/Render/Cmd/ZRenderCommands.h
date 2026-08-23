#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZCmdList;
    class ZRenderView;
    class ZRenderEntry;


    void CmdSetViewport(ZCmdList* pCmdList, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
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
    );
    void CmdScissorSetup(ZCmdList* pCmdList, ZRenderView* pRenderView, const ZVector4& vScissor, bool bUnk);
}
