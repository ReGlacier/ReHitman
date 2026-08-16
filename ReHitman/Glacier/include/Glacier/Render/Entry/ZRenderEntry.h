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
        {
            RT_SPRITE = 0x0,
            RT_GEOM = 0x1,
            RT_CAMERA = 0x2,
            RT_LIGHT = 0x3,
            RT_ARRAY = 0x4,
            RT_DEFORMER = 0x5,
            RT_ROOM = 0x6,
            RT_REFLECTOR = 0x7,
            RT_ENVSAMPLER = 0x8,
        };

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
        {
            ZBaseGeom* m_pBaseGeom;
            const ZRenderEntry* m_pOwner;
        };

        // vtbl
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
        virtual uint32_t GetHiddenBoneIndices() const;
        virtual uint32_t GetBoneIndexMask(const SPrimObject* pPrimObject);
        virtual float* GetPlanes() const;
        virtual void CalcLODMask(SRenderEntryNotifyInfo* pEntry);
        virtual void GetAttachedBaseGeoms(ZStackArray<1024, ZRenderEntry::ZAttachedBaseGeom>* pArray);

        // methods
        ZRenderEntry();
        void* AllocateMemory(uint32_t lSize);
        void SetObjectToWorldMatrix(const ZMatrix& mMatrix);
        bool AddRenderEntryInstances(SRenderEntryInstance** pInstances, uint32_t lNumRenderEntryInstances);

        // members
        RE_ADD_PADDING(0x10);                           // 0x04 - 0x14. TODO: Unknown; never initialized in ctors and no reads found in examined code paths
        uint32_t m_unk14;                               // 0x14. TODO: Unknown; zeroed in ZRenderEntry ctor (PC, inlined in ZRenderEntrySprite::Ctor & ZRenderEntryGeom::Ctor)
        RE_ADD_PADDING(0x10);                           // 0x18 - 0x28. TODO: Unknown; never initialized in ctors and no reads found in examined code paths
        uint8_t m_lLODLevelsActive;                     // 0x28. Verified by ZRenderEntry::EndFrame & ZRenderEntryGeom::Notify (PC). Name from XBox MiniNinjas PDB
        uint8_t m_lLODLevelsWanted;                     // 0x29. Verified by ZRenderEntry::EndFrame & ZRenderEntrySprite::Notify (PC)
        uint8_t m_lVariantId;                           // 0x2A. Verified by ZRenderEntryGeom::Notify & ZRenderEntryBones ctor (PC). Name from XBox MiniNinjas PDB
        uint8_t m_lEntryListsMask;                      // 0x2B. Verified by ZRenderEntryLists::Add & ZRenderEntry dtor (PC). Name from XBox MiniNinjas PDB
        uint8_t m_unk2C;                                // 0x2C. TODO: Unknown; zeroed in ZRenderEntry ctor (PC). XBox candidates: m_lChildView / m_lExtraFlags
        uint8_t m_lDrawDestinationOverride;             // 0x2D. Verified by ZRenderEntry::InitRenderEntryInstance & ZRenderEntryGeom ctor (PC). Name from XBox MiniNinjas PDB
        uint8_t m_lGeomListsControl;                    // 0x2E. Bitmask of (1 << ZRenderEntryLists::LISTTYPES). Verified by ZRenderEntryLists::Add & ZRenderEntryGeom ctor (PC). Name from XBox MiniNinjas PDB
        uint8_t m_lFade;                                // 0x2F. 0xFF = no fade, else m_fObjectFade = m_lFade * (1.f / 254.f) * 0.85f + 0.15f clamped to 1.f. Verified by ZRenderEntryGeom::SetRenderContext & ZRenderEntryBones::SetRenderContext (PC)
        uint16_t m_lControl;                            // 0x30. Verified by ZRenderEntry::SetObjectToWorldMatrix & ZRenderEntry ctor
        uint16_t m_lNumRenderEntryInstances;            // 0x32. Verified by ZRenderEntrySprite::SetSortValue & ZRenderEntry ctor
        SRenderEntryInstance* m_RenderEntryInstanceTable[4]; // 0x34 - 0x44. Inline storage for up to 4 instances. Verified by ZRenderEntry::AllocateMemory (00477420) & ZRenderEntry dtor (PC). Name from XBox MiniNinjas PDB
        SRenderEntryInstance** m_pRenderEntryInstances; // 0x44. Verified by ZRenderEntrySprite::SetSortValue & ZRenderEntry ctor
        ZMatrix m_ObjectToWorldMatrix;                  // 0x48. Verified by ZRenderEntry::SetObjectToWorldMatrix & ZRenderEntry ctor
        const ZBaseGeom* m_pEnvironment;                // 0x78. Verified by ZRenderDraw::CreateRenderEntries & ZRenderEntry::EndFrame (PC). Name from XBox MiniNinjas PDB
        const SDrawArray* m_pDrawArray;                 // 0x7C. Verified by ZRenderDraw::AddRenderEntryArray & ZRenderEntryBones::SetRenderContext (PC). Name from XBox MiniNinjas PDB (there it is m_pDrawArray[2], PC keeps a single pointer)
    };

    RE_VERIFY_OFFSET(ZRenderEntry, m_unk14, 0x14);                    // Verified by ZRenderEntry ctor (PC, inlined in ZRenderEntrySprite::Ctor & ZRenderEntryGeom::Ctor)
    RE_VERIFY_OFFSET(ZRenderEntry, m_lLODLevelsActive, 0x28);         // Verified by ZRenderEntry::EndFrame (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_lLODLevelsWanted, 0x29);
    RE_VERIFY_OFFSET(ZRenderEntry, m_lVariantId, 0x2A);               // Verified by ZRenderEntryGeom::Notify (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_lEntryListsMask, 0x2B);          // Verified by ZRenderEntryLists::Add (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_unk2C, 0x2C);                    // Verified by ZRenderEntry ctor (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_lDrawDestinationOverride, 0x2D); // Verified by ZRenderEntry::InitRenderEntryInstance (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_lGeomListsControl, 0x2E);        // Verified by ZRenderEntryLists::Add (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_lFade, 0x2F);                    // Verified by ZRenderEntryGeom::SetRenderContext (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_lControl, 0x30);                 // Verified by ZRenderEntry::SetObjectToWorldMatrix (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_lNumRenderEntryInstances, 0x32); // Verified by ZRenderEntrySprite::SetSortValue (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_RenderEntryInstanceTable, 0x34); // Verified by ZRenderEntry::AllocateMemory (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_pRenderEntryInstances, 0x44);    // Verified by ZRenderEntrySprite::SetSortValue (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_ObjectToWorldMatrix, 0x48);      // Verified by ZRenderEntry::SetObjectToWorldMatrix (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_pEnvironment, 0x78);             // Verified by ZRenderDraw::CreateRenderEntries (PC)
    RE_VERIFY_OFFSET(ZRenderEntry, m_pDrawArray, 0x7C);               // Verified by ZRenderDraw::AddRenderEntryArray (PC)
    RE_VERIFY_SIZE(ZRenderEntry, 0x80);                               // Verified by ZRenderEntrySprite ctor (PC, base part)
}