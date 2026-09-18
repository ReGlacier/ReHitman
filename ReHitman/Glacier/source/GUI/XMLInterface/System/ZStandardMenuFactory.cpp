#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGraphic.h>
#include <Glacier/GUI/XMLInterface/Elements/ZTextBlock.h>
#include <Glacier/GUI/XMLInterface/Elements/ZXMLFrame.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGUIToggle.h>
#include <Glacier/GUI/XMLInterface/Elements/ZCheckBox.h>
#include <Glacier/GUI/XMLInterface/Elements/ZToggleText.h>
#include <Glacier/GUI/XMLInterface/Elements/ZToggleTextSimple.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGUISlider.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGUISliderText.h>
#include <Glacier/GUI/XMLInterface/Elements/ZOpenWindow.h>
#include <Glacier/GUI/XMLInterface/Elements/ZOpenWindowTimer.h>
#include <Glacier/GUI/XMLInterface/Elements/ZCloseWindow.h>
#include <Glacier/GUI/XMLInterface/Elements/ZCloseAndDeleteTextMessage.h>
#include <Glacier/GUI/XMLInterface/Elements/ZCloseAndDeleteVoiceMessage.h>
#include <Glacier/GUI/XMLInterface/Elements/ZOpenSubwindow.h>
#include <Glacier/GUI/XMLInterface/Elements/ZQuitGame.h>
#include <Glacier/GUI/XMLInterface/Elements/ZAnimation.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGUIEdit.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGUITable.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGUIList.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGUIVirtualKeyboard.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/System/ZStandardMenuFactory.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>
#include <Glacier/GUI/XMLInterface/Windows/ZComponentWindow.h>
#include <Glacier/GUI/XMLInterface/Windows/ZStandardWindow.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    ZListDataInterface* ZStandardMenuFactory::GetListDataInterface(const char*)
    {
        return nullptr;
    }

    ITableDataInterface* ZStandardMenuFactory::GetTableDataInterface(const char*)
    {
        return nullptr;
    }

    IWindowInterface* ZStandardMenuFactory::WindowFactory(const char* pszType, ZMenuElements* pMenuElements)
    {
        if (strcmp(pszType, "Standard") == 0)
        {
            ZStandardWindow* pWindow = static_cast<ZStandardWindow*>(ZUniMemory::Allocate(sizeof(ZStandardWindow)));
            if (pWindow)
                new (pWindow) ZStandardWindow(pMenuElements);
            return pWindow;
        }

        if (strcmp(pszType, "Component") == 0)
        {
            ZComponentWindow* pWindow = static_cast<ZComponentWindow*>(ZUniMemory::Allocate(sizeof(ZComponentWindow)));
            if (pWindow)
                new (pWindow) ZComponentWindow(pMenuElements);
            return pWindow;
        }

        ZASSERT(false);
        return nullptr;
    }

    IGUIElement* ZStandardMenuFactory::GUIElementFactory(const char* pszType)
    {
        if (strcmp(pszType, "ZGuiEdit") == 0)
        {
            ZGUIEdit* pElement = static_cast<ZGUIEdit*>(ZUniMemory::Allocate(sizeof(ZGUIEdit)));
            if (pElement)
                new (pElement) ZGUIEdit();
            return pElement;
        }

        if (strcmp(pszType, "ZGUIVirtualKeyboard") == 0)
        {
            ZGUIVirtualKeyboard* pElement = static_cast<ZGUIVirtualKeyboard*>(ZUniMemory::Allocate(sizeof(ZGUIVirtualKeyboard)));
            if (pElement)
                new (pElement) ZGUIVirtualKeyboard();
            return pElement;
        }

        if (strcmp(pszType, "ToggleText") == 0)
        {
            ZToggleText* pElement = static_cast<ZToggleText*>(ZUniMemory::Allocate(sizeof(ZToggleText)));
            if (pElement)
                new (pElement) ZToggleText();
            return pElement;
        }

        if (strcmp(pszType, "ToggleTextSimple") == 0)
        {
            ZToggleTextSimple* pElement = static_cast<ZToggleTextSimple*>(ZUniMemory::Allocate(sizeof(ZToggleTextSimple)));
            if (pElement)
                new (pElement) ZToggleTextSimple();
            return pElement;
        }

        if (strcmp(pszType, "Slider") == 0)
        {
            ZGUISlider* pElement = static_cast<ZGUISlider*>(ZUniMemory::Allocate(sizeof(ZGUISlider)));
            if (pElement)
                new (pElement) ZGUISlider();
            return pElement;
        }

        if (strcmp(pszType, "SliderText") == 0)
        {
            ZGUISliderText* pElement = static_cast<ZGUISliderText*>(ZUniMemory::Allocate(sizeof(ZGUISliderText)));
            if (pElement)
                new (pElement) ZGUISliderText();
            return pElement;
        }

        if (strcmp(pszType, "CloseWindow") == 0)
        {
            ZCloseWindow* pElement = static_cast<ZCloseWindow*>(ZUniMemory::Allocate(sizeof(ZCloseWindow)));
            if (pElement)
                new (pElement) ZCloseWindow();
            return pElement;
        }

        if (strcmp(pszType, "CloseAndDeleteTextMessage") == 0)
        {
            ZCloseAndDeleteTextMessage* pElement =
                static_cast<ZCloseAndDeleteTextMessage*>(ZUniMemory::Allocate(sizeof(ZCloseAndDeleteTextMessage)));
            if (pElement)
                new (pElement) ZCloseAndDeleteTextMessage();
            return pElement;
        }

        if (strcmp(pszType, "CloseAndDeleteVoiceMessage") == 0)
        {
            ZCloseAndDeleteVoiceMessage* pElement =
                static_cast<ZCloseAndDeleteVoiceMessage*>(ZUniMemory::Allocate(sizeof(ZCloseAndDeleteVoiceMessage)));
            if (pElement)
                new (pElement) ZCloseAndDeleteVoiceMessage();
            return pElement;
        }

        if (strcmp(pszType, "OpenWindow") == 0)
        {
            ZOpenWindow* pElement = static_cast<ZOpenWindow*>(ZUniMemory::Allocate(sizeof(ZOpenWindow)));
            if (pElement)
                new (pElement) ZOpenWindow();
            return pElement;
        }

        if (strcmp(pszType, "OpenWindowTimer") == 0)
        {
            ZOpenWindowTimer* pElement = static_cast<ZOpenWindowTimer*>(ZUniMemory::Allocate(sizeof(ZOpenWindowTimer)));
            if (pElement)
                new (pElement) ZOpenWindowTimer();
            return pElement;
        }

        if (strcmp(pszType, "QuitGame") == 0)
        {
            ZQuitGame* pElement = static_cast<ZQuitGame*>(ZUniMemory::Allocate(sizeof(ZQuitGame)));
            if (pElement)
                new (pElement) ZQuitGame();
            return pElement;
        }

        if (strcmp(pszType, "TextBlock") == 0)
        {
            ZTextBlock* pElement = static_cast<ZTextBlock*>(ZUniMemory::Allocate(sizeof(ZTextBlock)));
            if (pElement)
                new (pElement) ZTextBlock();
            return pElement;
        }

        if (strcmp(pszType, "TextBlockSpeech") == 0)
        {
            ZTextBlockSpeech* pElement = static_cast<ZTextBlockSpeech*>(ZUniMemory::Allocate(sizeof(ZTextBlockSpeech)));
            if (pElement)
                new (pElement) ZTextBlockSpeech();
            return pElement;
        }

        if (strcmp(pszType, "Graphic") == 0)
        {
            ZGraphic* pElement = static_cast<ZGraphic*>(ZUniMemory::Allocate(sizeof(ZGraphic)));
            if (pElement)
                new (pElement) ZGraphic();
            return pElement;
        }

        if (strcmp(pszType, "OpenSubwindow") == 0)
        {
            ZOpenSubwindow* pElement = static_cast<ZOpenSubwindow*>(ZUniMemory::Allocate(sizeof(ZOpenSubwindow)));
            if (pElement)
                new (pElement) ZOpenSubwindow();
            return pElement;
        }

        if (strcmp(pszType, "Frame") == 0)
        {
            ZXMLFrame* pElement = static_cast<ZXMLFrame*>(ZUniMemory::Allocate(sizeof(ZXMLFrame)));
            if (pElement)
                new (pElement) ZXMLFrame();
            return pElement;
        }

        if (strcmp(pszType, "GUIList") == 0)
        {
            ZGUIList* pElement = static_cast<ZGUIList*>(ZUniMemory::Allocate(sizeof(ZGUIList)));
            if (pElement)
                new (pElement) ZGUIList();
            return pElement;
        }

        if (strcmp(pszType, "GUITable") == 0)
        {
            ZGUITable* pElement = static_cast<ZGUITable*>(ZUniMemory::Allocate(sizeof(ZGUITable)));
            if (pElement)
                new (pElement) ZGUITable();
            return pElement;
        }

        if (strcmp(pszType, "Animation") == 0)
        {
            ZAnimation* pElement = static_cast<ZAnimation*>(ZUniMemory::Allocate(sizeof(ZAnimation)));
            if (pElement)
                new (pElement) ZAnimation();
            return pElement;
        }

        if (strcmp(pszType, "GUIToggle") == 0)
        {
            ZGUIToggle* pElement = static_cast<ZGUIToggle*>(ZUniMemory::Allocate(sizeof(ZGUIToggle)));
            if (pElement)
                new (pElement) ZGUIToggle();
            return pElement;
        }

        if (strcmp(pszType, "CheckBox") == 0)
        {
            ZCheckBox* pElement = static_cast<ZCheckBox*>(ZUniMemory::Allocate(sizeof(ZCheckBox)));
            if (pElement)
                new (pElement) ZCheckBox();
            return pElement;
        }

        return nullptr;
    }
}
