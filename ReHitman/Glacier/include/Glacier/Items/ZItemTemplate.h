#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Items/ITEMSTATE.h>


namespace Glacier
{
    enum ITEMHANDS : uint32_t {
        IH_NONE = 0,
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
        virtual ZItem* CreateItem(ZGROUP* pGroup, unsigned int, bool bOverrideVisibleForNPC, bool bVisibleForNPC);
        virtual ZItem* CreateItemAndActuallyUseDestinationParameter(ZGROUP* group, unsigned int, bool isVisibleForNPC, bool);
        virtual uint32_t GetItemClassId() const;
        virtual void StateNotify(ZGEOM*, int);
        virtual void SetStates(CCom* pCom);
        virtual void ModifyState(CCom* pCom);
        virtual void SetStateGeometry(ZItem* pItem, ZGEOM* pGeom);
        virtual REFTAB* GetAvailableStates() const;
        virtual REFTAB* GetStates() const;
        virtual bool CheckStateExists(ITEMSTATE eState, const char* );
        virtual void FindStateGeoms(REFTAB* pRefTab, ITEMSTATE eState, const char* pszStateName);
        virtual void FindMainState(REFTAB* pRefTab);
        virtual ITEMHANDS GetItemHands() const;
        virtual void SetItemHands(ITEMHANDS);
        virtual ITEMSIZE GetItemSize() const;
        virtual ZGEOM* GetMainPos() const;
        virtual ZGEOM* GetCenterPos() const;
        virtual ZGEOM* GetCameraPos() const;
        virtual uint32_t GetMaterial() const;

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
    RE_VERIFY_SIZE(ZItemTemplate, 0x74); // Verified
    RE_VERIFY_OFFSET(ZItemTemplate, m_eItemHands, 0x4C);
}
