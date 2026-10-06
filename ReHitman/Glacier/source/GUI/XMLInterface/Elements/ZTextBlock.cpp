#include <Glacier/GUI/XMLInterface/Elements/ZTextBlock.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ResourceCollection.h>
#include <Glacier/Geom/ZSNDOBJ.h>
#include <Glacier/Audio/ZSoundObject.h>


namespace Glacier
{
    ZTextBlock::ZTextBlock()
    {
        m_pWinGroupText = nullptr;
        m_iLineSpace = 0;
        m_iWidth = 400;
    }

    ZTextBlock::~ZTextBlock()
    {
    }

    void ZTextBlock::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IGUIElement::readParams(ppParams, pElems);

        GuiOption::readString(m_sText, "Text", ppParams);
        GuiOption::readInt(m_iLineSpace, ppParams, "LineSpace");
        GuiOption::readInt(m_iWidth, ppParams, "Width");
    }

    ZGUIElementLink ZTextBlock::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        ZASSERT(m_pWinGroupText == nullptr);

        const ZVector2 vPos { pfPos[0], pfPos[1] };
        m_pWinGroupText = pResourceManager->GetTextGroup(vPos, m_pColorSet, pGroup, 128, m_eFontType, m_bShadow, m_eAlignment);

        if (!m_pWinGroupText)
            return ZGUIElementLink();

        SetLineSpace(m_pWinGroupText, m_iLineSpace);
        SetLineWidth(m_pWinGroupText, m_iWidth);

        SetText();

        float aSize[2];
        GetSize(m_pWinGroupText, aSize);
        return ZGUIElementLink(aSize[0], aSize[1]);
    }

    void ZTextBlock::ReleaseResources(ZResourceManager* pResourceManager)
    {
        if (m_pWinGroupText)
        {
            pResourceManager->ReleaseTextGroup(m_pWinGroupText);
            m_pWinGroupText = nullptr;
        }
    }

    void ZTextBlock::SetText()
    {
        const char* pText = m_sText.c_str();
        if (*pText != '%')
        {
            pText = g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", pText);
        }

        zstring sText;
        g_pGameData->InsertIconsInText(sText, pText, false);
        m_pWinGroupText->SetText(sText.c_str());
    }

    ZTextBlockSpeech::ZTextBlockSpeech()
    {
        m_iSoundIdLocale = 0;
        m_iSoundRef = 0;
        m_bShowText = true;
    }

    ZTextBlockSpeech::~ZTextBlockSpeech()
    {
    }

    void ZTextBlockSpeech::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZTextBlock::readParams(ppParams, pElems);

        GuiOption::readBool(&m_bShowText, ppParams, "ShowText", false);
    }

    ZGUIElementLink ZTextBlockSpeech::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        if (!m_iSoundIdLocale)
        {
            zstring sResource = zstring("AllLevels/Interface/") + m_sText.c_str();
            if (g_pEngineData->m_pLocaleResources->HasResource(sResource.c_str()))
            {
                Resource resource;
                g_pEngineData->m_pLocaleResources->GetResource(sResource.c_str(), &resource);
                m_iSoundIdLocale = resource.GetSoundResourceId();
            }
        }

        if (m_iSoundIdLocale)
        {
            CCom* pSceneCom = g_pEngineData->GetSceneCom();
            int iTitleMusicRef = 0;
            pSceneCom->GetVal("TitleMusic", &iTitleMusicRef);
            ZGEOM* pGeom = ZGEOM::RefToPtr(static_cast<ZREF>(iTitleMusicRef));
            if (pGeom)
            {
                ZSoundObject* pTitleMusic = static_cast<ZSNDOBJ*>(pGeom)->GetSoundObject();
                if (pTitleMusic)
                    pTitleMusic->SetVolumeAttenuation(0.125f);
            }

            m_iSoundRef = pGroup->AddSound2d(static_cast<int>(m_iSoundIdLocale));

            ZSoundObject* pSoundObject = g_pEngineData->SRefToPtr(m_iSoundRef);
            if (pSoundObject)
                pSoundObject->SetPriority(8);
        }

        return ZTextBlock::Setup(pfPos, pResourceManager, pGroup);
    }

    void ZTextBlockSpeech::ReleaseResources(ZResourceManager* pResourceManager)
    {
        if (m_iSoundRef)
        {
            ZSoundObject* pSoundObject = g_pEngineData->SRefToPtr(m_iSoundRef);
            if (pSoundObject)
            {
                pSoundObject->Stop();
                ZUniMemory::Delete(pSoundObject);
            }
        }

        ZTextBlock::ReleaseResources(pResourceManager);
    }

    void ZTextBlockSpeech::SetText()
    {
        if (m_bShowText)
            ZTextBlock::SetText();
    }
}
