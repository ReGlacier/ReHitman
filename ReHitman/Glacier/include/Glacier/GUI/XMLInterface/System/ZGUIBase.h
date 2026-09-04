#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZStaticVector.h>


namespace Glacier
{
    // fwds
    class ZMenuElements;
    class ZWINGROUP;
    class ZWINOBJ;
    class ZCONTROL;

    enum ENavigation
    {
        EVERTICAL = 0,
        EHORIZONTAL = 1,
    };

    enum EAlignment
    {
        ERIGHT = 0,
        ELEFT = 1,
        ECENTER = 2,
    };

    class ZGUIBase
    {
    public:
        static constexpr int32_t NAME_SIZE = 64;

        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems);
        virtual void addElement(const char* pName, ZGUIBase* pEntry);

        // methods
        ZGUIBase();
        const char* GetName() const;
        void SetName(const char* pName);
        void GetPos(Glacier::Vector2* pPos) const;
        bool HasAbsolutePos() const;

        // members
        char m_szName[NAME_SIZE] { 0 }; // +0x04 - Verified by PC Ctor
        Glacier::ZVector2 m_v2Pos {}; // +0x44 - Verified by PC Ctor

    protected:
        bool ReadNavigation(ENavigation& eNav, const char** ppAttrs);
        void GetSize(ZWINGROUP* pGroup, float* pSize) const;
        void SetChecked(bool bChecked, ZStaticVector<ZWINOBJ*, 8>& rUnchecked, ZStaticVector<ZWINOBJ*, 8>& rChecked);
        void SetStr(char* pDest, const char* pSrc);
        bool ReadElementAlignment(EAlignment& eAlignment, const char** ppAttrs, const char* pAttrName);
        bool ReadType(uint32_t& iType, const char** ppAttrs);
        void LinkHorizontal(ZCONTROL* pCtrl1, ZCONTROL* pCtrl2);
        void LinkVertical(ZCONTROL* pCtrl1, ZCONTROL* pCtrl2);
    };
    RE_VERIFY_SIZE(ZGUIBase, 0x4C); // Verified
    RE_VERIFY_OFFSET(ZGUIBase, m_szName, 0x04);
    RE_VERIFY_OFFSET(ZGUIBase, m_v2Pos, 0x44);
}
