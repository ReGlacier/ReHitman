#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/GUI/Control/ZCONTROL.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <cstring>


namespace Glacier
{
    // Forward declarations for XML reader free functions
    // TODO: Finish me after ZGUIXMLReader2 reversed
    void ReadText(char* pDest, const char* pAttrName, const char** ppAttrs);
    bool ReadV2(float* pDest, const char** ppAttrs, const char* pDefaultAttr);
    const char* GetAttr(const char** ppAttrs, const char* pAttrName, const char* pDefault);


    ZGUIBase::ZGUIBase()
    {
        SetStr(m_szName, "Not_Initialized");
        m_v2Pos.x = -5000.0f;
        m_v2Pos.y = -5000.0f;
    }

    void ZGUIBase::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ReadText(m_szName, "Name", ppParams);
        ReadV2(&m_v2Pos.x, ppParams, nullptr);
    }

    void ZGUIBase::addElement(const char* pName, ZGUIBase* pEntry)
    {
        // Base implementation does nothing
    }

    const char* ZGUIBase::GetName() const
    {
        return m_szName;
    }

    void ZGUIBase::SetName(const char* pName)
    {
        SetStr(m_szName, pName);
    }

    void ZGUIBase::GetPos(Glacier::Vector2* pPos) const
    {
        pPos->x = m_v2Pos.x;
        pPos->y = m_v2Pos.y;
    }

    bool ZGUIBase::HasAbsolutePos() const
    {
        return m_v2Pos.x != -5000.0f || m_v2Pos.y > -5000.0f;
    }

    void ZGUIBase::GetSize(ZWINGROUP* pGroup, float* pSize) const
    {
        // TODO: Finish me (complex geometry traversal - see PC 0x564040 / PS2 0x335C24)
        pSize[0] = 0.0f;
        pSize[1] = 0.0f;
    }

    bool ZGUIBase::ReadNavigation(ENavigation& eNav, const char** ppAttrs)
    {
        const char* pAttr = GetAttr(ppAttrs, "Navigation", nullptr);
        if (!pAttr)
            return false;

        static const struct
        {
            const char* pName;
            ENavigation eValue;
        } s_aNavigation[] = {
            { "Horizontal", EVERTICAL },
            { "Vertical", EHORIZONTAL },
        };

        for (int i = 0; i < 2; ++i)
        {
            if (strcmp(s_aNavigation[i].pName, pAttr) == 0)
            {
                eNav = s_aNavigation[i].eValue;
                return true;
            }
        }

        return false;
    }

    void ZGUIBase::SetStr(char* pDest, const char* pSrc)
    {
        ZASSERT(strlen(pSrc) < NAME_SIZE);
        strcpy(pDest, pSrc);
    }

    bool ZGUIBase::ReadElementAlignment(EAlignment& eAlignment, const char** ppAttrs, const char* pAttrName)
    {
        const char* pAttr = GetAttr(ppAttrs, pAttrName, nullptr);
        if (!pAttr)
            return false;

        static const struct
        {
            const char* pName;
            EAlignment eValue;
        } s_aAlignment[] = {
            { "Left", ELEFT },
            { "Right", ERIGHT },
            { "Center", ECENTER },
        };

        for (int i = 0; i < 3; ++i)
        {
            if (strcmp(s_aAlignment[i].pName, pAttr) == 0)
            {
                eAlignment = s_aAlignment[i].eValue;
                return true;
            }
        }

        return false;
    }

    bool ZGUIBase::ReadType(uint32_t& iType, const char** ppAttrs)
    {
        // TODO: Finish me (requires ZGUIBase type flag table - see PS2 0x336330)
        return false;
    }

    void ZGUIBase::SetChecked(bool bChecked, ZStaticVector<ZWINOBJ*, 8>& rUnchecked, ZStaticVector<ZWINOBJ*, 8>& rChecked)
    {
        // TODO: Finish me (requires ZWINOBJ vtable methods - see PS2 0x336454)
    }

    void ZGUIBase::LinkHorizontal(ZCONTROL* pCtrl1, ZCONTROL* pCtrl2)
    {
        // TODO: Finish me (see PS2 LinkVertical for pattern - PC not found)
    }

    void ZGUIBase::LinkVertical(ZCONTROL* pCtrl1, ZCONTROL* pCtrl2)
    {
        // TODO: Finish me (see PS2 0x3366C4 for reference)
    }
}
