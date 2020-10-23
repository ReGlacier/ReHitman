#pragma once

namespace Hitman::BloodMoney
{
    // Type forwardings for class ZUpgradeInfo
    class ZGUIBase;
    class ZXMLGUISystem;
    class ZWINGROUP;
    enum class eZWUserEvents;
    class ZResourceManager;

    // Class definition ZUpgradeInfo
    class ZUpgradeInfo {
        virtual void addElement(char const*, ZGUIBase*); //#0000 at 0044D9CC org ZGUIBase::addElement(char const*,ZGUIBase*)
        virtual void Update(bool); //#0001 at 003C59C8 org ZUpgradeInfo::Update(bool)
        virtual void Click(eZWUserEvents,int,ZXMLGUISystem *); //#0002 at 0044DA68 org IGUIElement::Click(eZWUserEvents,int,ZXMLGUISystem *)
        virtual void OpenSubwindow(); //#0003 at 0044DA18 org IGUIElement::OpenSubwindow(void)
        virtual void SetValue(int); //#0004 at 0044DA20 org IGUIElement::SetValue(int)
        virtual void Use(); //#0005 at 0044DA28 org IGUIElement::Use(void)
        virtual void FocusUpdate(); //#0006 at 0044DA30 org IGUIElement::FocusUpdate(void)
        virtual void Setup(float *,ZResourceManager *,ZWINGROUP *); //#0007 at 003C5A80 org ZUpgradeInfo::Setup(float *,ZResourceManager *,ZWINGROUP *)
        virtual void ReleaseResources(ZResourceManager *); //#0008 at 003C5EC0 org ZUpgradeInfo::ReleaseResources(ZResourceManager *)
        virtual void SetFocus(bool); //#0009 at 0044DA60 org IGUIElement::SetFocus(bool)
        virtual void Cancel(); //#0010 at 0044DA38 org IGUIElement::Cancel(void)
        virtual void Invalidate(); //#0011 at 0044DA40 org IGUIElement::Invalidate(void)
        virtual void GetWindow(); //#0012 at 0044DA48 org IGUIElement::GetWindow(void)
    }; //End of ZUpgradeInfo from 0057B880
}