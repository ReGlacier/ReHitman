#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZGROUP.h>

namespace Glacier
{
    class ZItemTemplate : public ZGROUP
    {
    public:
        // vftable
        virtual ZItem* CreateItem(ZGROUP*, unsigned int, bool isVisibleForNPC, bool);
        virtual ZItem* CreateItemAndActuallyUseDestinationParameter(ZGROUP* group, unsigned int, bool isVisibleForNPC, bool);
        virtual int GetItemClassId();
        virtual void StateNotify(ZGEOM*, int);
        virtual void SetStates(CCom*);
        virtual void ModifyState(CCom*);
        virtual void SetStateGeometry(ZItem*, ZGEOM*);
        virtual REFTAB* GetAvailableStates();
        virtual ITEMSTATE* GetStates();
        virtual bool CheckStateExists(ITEMSTATE, const char* );
        virtual void FindStateGeoms(REFTAB* reftab, ITEMSTATE, const char* stateName);
        virtual void FindMainState(REFTAB* reftab);
        virtual void GetItemHands();
        virtual void SetItemHands(ITEMHANDS);
        virtual int GetItemSize();
        virtual ZGEOM* GetMainPos();
        virtual ZGEOM* GetCenterPos();
        virtual ZGEOM* GetCameraPos();
        virtual void* GetMaterial();

        // data (total size is 0x74, ZGROUP size is 0x4C)
        int m_itemInHands;
        int m_itemSize;
        int m_field54;
        int m_field58;
        int m_field5C;
        int m_field60;
        REFTAB* m_states;
        Glacier::ZMSGID m_MSG_ITEMSETSTATE;
        Glacier::ZMSGID m_MSG_ITEMGETANIMNAME;
        int m_material;
        int m_field70;
    };

}