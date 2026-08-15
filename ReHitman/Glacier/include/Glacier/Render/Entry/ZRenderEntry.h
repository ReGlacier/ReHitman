#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderEntry
    {
    public:
        // types
        enum RENDERENTRY_BASETYPE
        {};

        // Flags of m_lControl. Names from XBox MiniNinjas PDB; on PC verified only RE_HASMOVED (ZRenderEntry::SetObjectToWorldMatrix)
        enum RENDERENTRY_FLAGS : uint16_t
        {
            RE_NOTIFIED             = 0x1,
            RE_HASBONES             = 0x2,
            RE_ADDTOREUSE           = 0x4,
            RE_ISSPRITE             = 0x8,
            RE_NEEDUPDATE           = 0x10,
            RE_UPDATELIGHT          = 0x20,
            RE_CREATEDTHISFRAME     = 0x40,
            RE_ISACTOR              = 0x80,
            RE_HASMOVED             = 0x100,
            RE_WANT_VIEW_NOTIFY     = 0x200,
            RE_ATTACH_UPDATE        = 0x400,
            RE_STATIC_SHADOW_SUB    = 0x800,
            RE_CONSTRUCTION_FAILED  = 0x1000,
            RE_GOT_DRAWUPDATE       = 0x2000,
            RE_ISUPDATINGATTACHED   = 0x4000,
            RE_ISBACKDROP           = 0x8000,
        };

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

        void SetObjectToWorldMatrix(const ZMatrix& mMatrix);

        // members
        // TODO: Finish me
        RE_ADD_PADDING(0x2C);                           // 0x04 - 0x30
        uint16_t m_lControl;                            // 0x30. Verified by ZRenderEntry::SetObjectToWorldMatrix & ZRenderEntry ctor
        uint16_t m_lNumRenderEntryInstances;            // 0x32. Verified by ZRenderEntrySprite::SetSortValue & ZRenderEntry ctor
        RE_ADD_PADDING(0x10);                           // 0x34 - 0x44
        SRenderEntryInstance** m_pRenderEntryInstances; // 0x44. Verified by ZRenderEntrySprite::SetSortValue & ZRenderEntry ctor
        ZMatrix m_ObjectToWorldMatrix;                  // 0x48. Verified by ZRenderEntry::SetObjectToWorldMatrix & ZRenderEntry ctor
        RE_ADD_PADDING(0x8);                            // 0x78 - 0x80
    };

    RE_VERIFY_OFFSET(ZRenderEntry, m_lControl, 0x30);              // Verified by ZRenderEntry::SetObjectToWorldMatrix (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_lNumRenderEntryInstances, 0x32); // Verified by ZRenderEntrySprite::SetSortValue (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_pRenderEntryInstances, 0x44); // Verified by ZRenderEntrySprite::SetSortValue (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_ObjectToWorldMatrix, 0x48);   // Verified by ZRenderEntry::SetObjectToWorldMatrix (PC)
    RE_VERIFY_SIZE(ZRenderEntry, 0x80);                            // Verified by ZRenderEntrySprite ctor (PC, base part)
}