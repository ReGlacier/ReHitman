#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/GUI/ZWINPIC.h>
#include <Glacier/GUI/ZLINEOBJ.h>
#include <Glacier/GUI/Control/ZCONTROL.h>
#include <Glacier/GUI/Font/ZFONT.h>
#include <Glacier/GUI/eZWUserEvents.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/zstring.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    namespace GUI
    {
        const char* GetAttr(const char** ppAttrs, const char* pAttrName, bool bCaseInsensitive)
        {
            if (!*ppAttrs)
                return nullptr;

            for (int i = 0; ; i += 2)
            {
                const char* pKey = ppAttrs[i];
                if (!(bCaseInsensitive ? _stricmp(pKey, pAttrName) : strcmp(pKey, pAttrName)))
                    return ppAttrs[i + 1];

                if (!ppAttrs[i + 2])
                    return nullptr;
            }
        }

        bool ReadUIntHex(uint32_t& rDest, const char** ppAttrs, const char* pAttrName)
        {
            const char* pAttr = GUI::GetAttr(ppAttrs, pAttrName, false);
            if (!pAttr)
                return false;

            int iValue = 0;
            sscanf(pAttr, "%i", &iValue);
            rDest = static_cast<uint32_t>(iValue);
            return true;
        }
    }

    namespace ZGUI
    {
        bool ReadText(char* pDest, const char* pAttrName, const char** ppAttrs)
        {
            const char* pAttr = GUI::GetAttr(ppAttrs, pAttrName, false);
            if (!pAttr)
                return false;

            ZASSERT(strlen(pAttr) < 0x40);
            strcpy(pDest, pAttr);
            return true;
        }
    }

    namespace GuiOption
    {
        bool readBool(bool* pValue, const char** ppAttrs, const char* pAttrName, bool bCaseInsensitive)
        {
            const char* pAttr = GUI::GetAttr(ppAttrs, pAttrName, bCaseInsensitive);
            if (!pAttr)
                return false;

            if (strcmp("True", pAttr) == 0)
            {
                *pValue = true;
                return true;
            }

            if (strcmp("False", pAttr) != 0)
                return false;

            *pValue = false;
            return true;
        }

        bool readString(zstring& rDest, const char* pAttrName, const char** ppAttrs)
        {
            const char* pAttr = GUI::GetAttr(ppAttrs, pAttrName, false);
            if (!pAttr)
                return false;

            rDest = zstring(pAttr);
            return true;
        }

        bool readInt(int32_t& rDest, const char** ppAttrs, const char* pAttrName)
        {
            const char* pAttr = GUI::GetAttr(ppAttrs, pAttrName, false);
            if (!pAttr)
                return false;

            rDest = atoi(pAttr);
            return true;
        }

        bool readInt2(float& rDest, const char** ppAttrs, const char* pAttrName)
        {
            const char* pAttr = GUI::GetAttr(ppAttrs, pAttrName, false);
            if (!pAttr)
                return false;

            rDest = static_cast<float>(atof(pAttr));
            return true;
        }

        bool readVirtualKey(int32_t* pValue, const char** ppAttrs, const char* pAttrName)
        {
            const char* pAttr = GUI::GetAttr(ppAttrs, pAttrName, false);
            if (!pAttr || !g_pSysInterface->m_bUseGameController)
                return false;

            static const struct
            {
                const char* pName;
                int32_t iVirtualKey;
            } s_aKeys[] = {
                { "ZW_SELECT", eZW_SELECT },
                { "ZW_SELECT2", eZW_SELECT2 },
                { "ZW_SELECT3", eZW_SELECT3 },
                { "ZW_SELECT4", eZW_SELECT4 },
                { "ZW_CANCEL", 129 },
                { "Cross", eZW_SELECT },
                { "Circle", eZW_SELECT2 },
                { "Triangle", eZW_SELECT3 },
                { "Square", eZW_SELECT4 },
                { "Start", eZW_START },
            };

            for (const auto& sKey : s_aKeys)
            {
                if (strcmp(sKey.pName, pAttr) == 0)
                {
                    *pValue = sKey.iVirtualKey;
                    return true;
                }
            }

            return false;
        }

        bool readMoveButton(int32_t* pValue, const char** ppAttrs, const char* pAttrName)
        {
            const char* pAttr = GUI::GetAttr(ppAttrs, pAttrName, false);
            if (!pAttr)
                return false;

            static const struct
            {
                const char* pName;
                int32_t iButton;
            } s_aButtons[] = {
                { "Up", eZW_UP },
                { "Down", eZW_DOWN },
                { "Left", eZW_LEFT },
                { "Right", eZW_RIGHT },
            };

            for (const auto& sButton : s_aButtons)
            {
                if (strcmp(sButton.pName, pAttr) == 0)
                {
                    *pValue = sButton.iButton;
                    return true;
                }
            }

            return false;
        }
    }

    bool ReadV2(float (&pDest)[2], const char** ppAttrs, const char* pDefaultAttrPrefix)
    {
        static const char* s_aComponentName[2] = { "X", "Y" };

        for (int i = 0; i < 2; ++i)
        {
            zstring sAttrName;
            if (pDefaultAttrPrefix)
                sAttrName.format("%s%s", pDefaultAttrPrefix, s_aComponentName[i]);
            else
                sAttrName = zstring(s_aComponentName[i]);

            const char* pAttr = GUI::GetAttr(ppAttrs, sAttrName.c_str(), false);
            if (pAttr)
                pDest[i] = static_cast<float>(atof(pAttr));
        }

        return true;
    }


    bool ReadAlignment(uint32_t& rAlignment, const char** ppAttrs)
    {
        bool bResult = false;

        const char* pAlignH = GUI::GetAttr(ppAttrs, "AlignH", false);
        if (pAlignH)
        {
            uint32_t iAlignH = 0;
            if (strcmp(pAlignH, "Left") == 0)
                iAlignH = 0x01u;
            else if (strcmp(pAlignH, "Right") == 0)
                iAlignH = 0x02u;
            else if (strcmp(pAlignH, "Center") == 0)
                iAlignH = 0x04u;

            if (iAlignH)
            {
                rAlignment = iAlignH | (rAlignment & 0xF0u);
                bResult = true;
            }
        }

        const char* pAlignV = GUI::GetAttr(ppAttrs, "AlignV", false);
        if (pAlignV)
        {
            uint32_t iAlignV = 0;
            if (strcmp(pAlignV, "Top") == 0)
                iAlignV = 0x10u;
            else if (strcmp(pAlignV, "Bottom") == 0)
                iAlignV = 0x20u;
            else if (strcmp(pAlignV, "Center") == 0)
                iAlignV = 0x40u;

            if (iAlignV)
            {
                rAlignment = iAlignV | (rAlignment & 0x0Fu);
                bResult = true;
            }
        }

        const char* pAlignMode = GUI::GetAttr(ppAttrs, "AlignMode", false);
        if (pAlignMode)
        {
            if (strcmp(pAlignMode, "Border") == 0)
            {
                rAlignment |= 0x100u;
                bResult = true;
            }
            else if (strcmp(pAlignMode, "Position") == 0)
            {
                rAlignment &= 0xFFFFFEFFu;
                bResult = true;
            }
        }

        return bResult;
    }

    ZGUIBase::ZGUIBase()
    {
        SetStr(m_szName, "Not_Initialized");
        m_v2Pos.x = -5000.0f;
        m_v2Pos.y = -5000.0f;
    }

    void ZGUIBase::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZGUI::ReadText(m_szName, "Name", ppParams);
        ReadV2(reinterpret_cast<float(&)[2]>(m_v2Pos), ppParams, nullptr);
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
        pSize[0] = 0.0f;
        pSize[1] = 0.0f;

        if (!pGroup)
            return;

        for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (!pGeom || !pGeom->IsDerivedFrom<ZWINOBJ>())
                continue;

            ZVector3 vGeomSize;
            pGeom->GetSize(vGeomSize);
            float fWidth = vGeomSize.x + vGeomSize.x;
            float fHeight = vGeomSize.y + vGeomSize.y;

            ZVector3 vPos;
            pGeom->GetPos(vPos);

            if (const ZWINPIC* pWinPic = geom_cast<ZWINPIC>(pGeom))
            {
                fWidth = static_cast<float>(pWinPic->m_iOriginalSizeX);
                fHeight = static_cast<float>(pWinPic->m_iOriginalSizeY);
            }

            if (ZLINEOBJ* pLineObj = geom_cast<ZLINEOBJ>(pGeom))
            {
                ZFONT* pFont = pLineObj->GetFont();
                if (pLineObj->m_iNumberOfLines < 2)
                {
                    fHeight = static_cast<float>(pFont->GetMaxHeight());
                }
                else
                {
                    fHeight = static_cast<float>(pLineObj->m_iNumberOfLines * pLineObj->GetLineHeight());
                }
            }

            const uint8_t dwAlignment = static_cast<ZWINOBJ*>(pGeom)->GetAlignment();

            if (dwAlignment & 0x01)
                fWidth += vPos.x;
            else if (dwAlignment & 0x02)
                fWidth = vPos.x;
            else if (dwAlignment & 0x04)
                fWidth = fWidth * 0.5f + vPos.x;
            else
                ZASSERT(false);

            if (dwAlignment & 0x20)
                fHeight = vPos.y;
            else if (dwAlignment & 0x40)
                fHeight = fHeight * 0.5f + vPos.y;
            else
                fHeight += vPos.y;

            if (pSize[0] < fWidth)
                pSize[0] = fWidth;

            if (pSize[1] < fHeight)
                pSize[1] = fHeight;
        }
    }

    bool ZGUIBase::ReadNavigation(ENavigation& eNav, const char** ppAttrs)
    {
        const char* pAttr = GUI::GetAttr(ppAttrs, "Navigation", false);
        if (!pAttr)
            return false;

        static const struct
        {
            const char* pName;
            ENavigation eValue;
        } s_aNavigation[] = {
            { "Horizontal", EHORIZONTAL },
            { "Vertical", EVERTICAL },
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
        const char* pAttr = GUI::GetAttr(ppAttrs, pAttrName, false);
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
        const char* pAttr = GUI::GetAttr(ppAttrs, "Value", false);
        if (!pAttr)
            return false;

        static const struct
        {
            const char* pName;
            ZWINOBJ::EType eType;
        } s_aTypes[] = {
            { "Focus", ZWINOBJ::TYPE_Focused },
            { "Normal", ZWINOBJ::TYPE_Normal },
            { "Pushed", ZWINOBJ::TYPE_Pushed },
            { "Background", ZWINOBJ::TYPE_Background },
            { "Disabled", ZWINOBJ::TYPE_Disabled },
            { "Hover", ZWINOBJ::TYPE_Hover },
        };

        for (const auto& sType : s_aTypes)
        {
            if (strcmp(sType.pName, pAttr) == 0)
            {
                iType |= 1u << sType.eType;
                return true;
            }
        }

        return false;
    }

    void ZGUIBase::SetChecked(bool bChecked, ZStaticVector<ZWINOBJ*, 8>& rUnchecked, ZStaticVector<ZWINOBJ*, 8>& rChecked)
    {
        ZStaticVector<ZWINOBJ*, 8>& rActive = bChecked ? rChecked : rUnchecked;
        ZStaticVector<ZWINOBJ*, 8>& rInactive = bChecked ? rUnchecked : rChecked;

        for (uint32_t i = 0; i < rActive.size(); ++i)
            rActive[i]->MakeActive();

        for (uint32_t i = 0; i < rInactive.size(); ++i)
            rInactive[i]->MakeInactive();
    }

    void ZGUIBase::LinkHorizontal(ZCONTROL* pCtrl1, ZCONTROL* pCtrl2)
    {
        if (pCtrl1 && pCtrl2)
        {
            pCtrl1->SetNextFocus(pCtrl2, Right);
            pCtrl2->SetNextFocus(pCtrl1, Left);
        }
    }

    void ZGUIBase::LinkVertical(ZCONTROL* pCtrl1, ZCONTROL* pCtrl2)
    {
        if (pCtrl1 && pCtrl2)
        {
            pCtrl1->SetNextFocus(pCtrl2, Down);
            pCtrl2->SetNextFocus(pCtrl1, Up);
        }
    }
}
