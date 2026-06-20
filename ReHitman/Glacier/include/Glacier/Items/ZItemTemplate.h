#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>

namespace Glacier
{
    enum ITEMHANDS : uint32_t {
        eIH_NONE = 0, 
        IH_ONEHANDED = 1,
        IH_TWOHANDED = 2, 
        IH_FORCE32 = 2147483647u
    };

    enum ITEMSIZE : uint32_t {
        eITEMSIZE_SMALL = 0,
        ITEMSIZE_LARGE  = 1,
        ITEMSIZE_FORCE32 = 2147483647u
    };

    class ZItemTemplate : public ZGROUP
    {
    public:
        // static consts
        static constexpr uint32_t kItemGroupId = 0x4954454D;

        // vftable
        virtual ZItem* CreateItem(ZGROUP*, unsigned int, bool bOverrideVisibleForNPC, bool bVisibleForNPC);
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
        ITEMHANDS m_eItemHands;
        ITEMSIZE m_eItemSize;
        bool m_bIsVisibleToNPCs;
        RE_ADD_PADDING(3);
        uint32_t m_rInventoryPicture;
        uint32_t m_rPointerPicture;
        uint32_t m_rPointerContextPicture;
        REFTAB* m_pStates;
        uint16_t m_msgSetItemState;
        uint16_t m_msgGetItemSettings;
        uint32_t m_rMaterial;
        bool m_bSendImpactEvent;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZItemTemplate, 0x74);
}