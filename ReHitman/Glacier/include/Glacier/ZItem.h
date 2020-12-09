#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZRTTI.h>
#include <Glacier/ZMath.h>
#include <Glacier/ZGROUP.h>

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
        virtual void Place(float const*,float const*);
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
        int m_field4C;
        int m_field50;
        bool m_isVisibleForNPCs;
        bool m_field55;
        bool m_field56;
        bool m_field57;
        int m_field58;
        int m_field5C;
        int m_field60;
        int m_field64;
        int m_field68;
        int m_field6C;
        int m_field70;
        int m_field74;
        int m_field78;
        int m_field7C;
        int m_field80;
    };
}