#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Items/ITEMSTATE.h>

namespace Glacier
{
    class ZItem : public ZGROUP
    {
    public:
        // vftable
        virtual void CreateFromTemplate();
        virtual void GetItemRootTM(float *,float *);
        virtual void GetMainItemRootTM(float *,float *);
        virtual void GetState();
        virtual void SetState(ITEMSTATE, CCom*);
        virtual void Place(float const*, float const*);
        virtual void SetMain(uint);
        virtual void GetMain();
        virtual void GetMainMatPos(float *,float *,uint);
        virtual void SetItemTemplate(Glacier::ZREF itemTemplateRef);
        virtual ZItemTemplate* GetItemTemplate();
        virtual void VerifyItemTemplate(ZItemTemplate const*);
        virtual void SetItemOwner(uint,ZGROUP *,bool,bool);
        virtual void GetItemOwner();
        virtual void GetAction(uint);
        virtual void* InitPickup();
        virtual void EnablePickup(bool);
        virtual void OnMoved();
        virtual void OnMoving();
        virtual void Delete();
        virtual void Clear(uint);
        virtual ZGEOM* GetMarkedGeom(char const*);
        virtual void AddActivate(ZItemState *,float);
        virtual void AddDeactivate(uint, float);
        virtual void UpdateActivate();
        virtual void UpdateDeactivate();

        // data (total size is 0x84, ZGROUP size is 0x4C)
        ITEMSTATE m_lCurrentState;
        uint32_t m_rItemTemplate;
        bool m_bVisibleToNPCs;
        RE_ADD_PADDING(3);
        uint32_t m_rItemOwner;
        uint32_t m_rMain;
        ZPoolAllocRefTab* m_pStateRemove;
        ZPoolAllocRefTab* m_pStateReuse;
        ZPoolAllocLinkSortRefTab* m_pDeactivateStates;
        ZPoolAllocLinkSortRefTab* m_pActivateStates;
        uint16_t m_msgSetItemState;
        uint16_t m_msgGetAvailableStates;
        uint16_t m_msgSetItem;
        RE_ADD_PADDING(2);
        uint32_t m_iVisionID;
        float m_fLastUpdatedPosition;
        bool m_bInMotion;
        bool m_NewItem;
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(ZItem, 0x84); // Verified
}