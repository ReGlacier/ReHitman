#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderEntry
    {
    public:
        // types
        enum RENDERENTRY_BASETYPE
        {};

        struct ZAttachedBaseGeom
        {};

        // vtbl (confirmed by iOS build; partially confirmed with PC)
        virtual ~ZRenderEntry();
        virtual void Notify(const SRenderEntryNotifyInfo* pEntry);
        virtual void Update();
        virtual void AttachUpdate();
        virtual void GetVisible(ZCmdList* pCmdList, ZRenderEntryGeom* pGeomEntry, ZViewSpace* pViewSpace, ZRenderView* pView, ZRenderEntryLists* pEntryList);
        virtual uint32_t GetInstanceRepeat(const ZRenderObjectInstance* pObjInstance);
        virtual void SetRenderContext(ZRenderContext* pContext, const ZRenderObjectInstance* pObjInstance);
        virtual void EndFrame();
        virtual void GetDrawInstances(SRenderEntryInstance** ppEntries, SRenderEntryNotifyInfo* pEntry);
        virtual RENDERENTRY_BASETYPE GetType() const = 0;
        virtual ZBaseGeom* GetBaseGeom() const;
        virtual uint32_t GetPrim() const;
        virtual void* GetHiddenBoneIndices() const;
        virtual uint32_t GetBoneIndexMask(const SPrimObject* pPrimObject);
        virtual float* GetPlanes() const;
        virtual void CalcLODMask(SRenderEntryNotifyInfo* pEntry);
        virtual void GetAttachedBaseGeoms(ZStackArray<1024, ZRenderEntry::ZAttachedBaseGeom>* pArray);

        // methods
        // members
        // TODO: Finish me
    };
}