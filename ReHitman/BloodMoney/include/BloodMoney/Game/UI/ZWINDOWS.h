#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZWinEvents.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/LINKREFTAB.h>
#include <Glacier/ZSTL/LINKSORTREFTAB.h>

#include <BloodMoney/Game/UI/UIFWD.h>
#include <BloodMoney/Game/UI/ZWinInput.h>
#include <BloodMoney/Game/UI/ZWINGROUP.h>

namespace Hitman::BloodMoney
{
    class ZWINDOWS : public ZWINGROUP
    {
    public:
        class EventSubscriber //I'm not sure that is correct, need to reverse it
        {
        public:
            Glacier::ZREF m_groupREF; //0x0000
            int m_subId; //0x0004 (always 0x20000)
            int m_eventID; //0x0008
            bool m_flag_0C; //0x000C
            bool m_field_0D; //0x000D
            bool m_field_0E; //0x000E
            bool m_field_0F; //0x000F
            float fArg;
            int m_field14;
            int m_field18;
            int m_field1C;
        }; //Size: 0x00E4

        /// vftable
        virtual bool Notify(Glacier::ZWM_MESSAGE message, int evData1, int evData2, ZWINGROUP* group); //Note: evData1 & evData2 not used
        virtual bool Notify(Glacier::ZWMEVENT* event, ZWINGROUP* group);
        virtual bool NotifySubscribers(Glacier::ZWMEVENT*);
        virtual void CheckCommands();
        virtual void SetFocusedControl(ZWINGROUP*);
        virtual ZWINGROUP* GetFocusedControl();
        virtual void ReleaseFocusedControl();
        virtual void PushWindow(unsigned int id);
        virtual void PushWindow(ZWINDOW* pWindow);
        virtual void PopWindow();
        virtual void SetMousePos(Glacier::ZVector2* position);
        virtual Glacier::ZVector2* GetMousePos();
        virtual void SetShowMouse(bool);
        virtual void UpdateMouse();
        virtual void SetMouseSpeed(float);
        virtual void SetMousePosition(const Glacier::ZVector2*);
        virtual void SetMouseDeltaPos(const Glacier::ZVector2*);
        virtual Glacier::ZGEOM* GetMouseGeom();
        virtual void ForceUpdateMouse();
        virtual void AddCamera(Glacier::ZCAMERA*);
        virtual void RemoveCamera(Glacier::ZCAMERA*);
        virtual void SetUseZBuffer(bool);
        virtual void SetClearScreen(bool);
        virtual void SetBackColor(int r, int g, int b);
        virtual void SetBackColor(int hex);
        virtual void SetGfxHandler(ZWinGfxHandler*);
        virtual ZWinGfxHandler* GetGfxHandler();
        virtual EventSubscriber* AddSubscriber(unsigned int id, int eventId, ZWINGROUP* group, float priority, bool flag0C);
        virtual void RemoveSubscriber(unsigned int id, int eventId, ZWINGROUP* group);

        //data (total size is 0x36C, ZWINGROUP size is 0x54)
        int m_focusedControl;
        int field_58;
        int m_mouseGEOM;
        Vec2F m_mousePos;
        int field_68;
        int field_6c;
        Vec2F field_70;
        int field_78;
        int field_7c;
        float m_mouseSpeed;
        int field_84;
        Glacier::LINKSORTREFTAB m_subscribers;
        Glacier::LINKREFTAB m_linkRefTab_b4;
        Glacier::REFTAB32 m_reftab32;
        int field_188;
        int field_18c;
        Glacier::LINKSORTREFTAB m_cameras;
        int field_1bc;
        int m_gfxHandler;
        int field_1c4;
        int field_1c8;
        int field_1cc;
        int field_1d0;
        int field_1d4;
        int field_1d8;
        char field_1dc;
        char field_1DD;
        bool m_mouseVisible;
        char field_1DF;
        bool m_shouldClearScreen;
        char m_forceUpdateMouse;
        char field_1E2;
        char field_1E3;
        int field_1e4;
        char field_1e8;
        char field_1E9;
        char field_1EA;
        char field_1EB;
        int field_1ec;
        ZWinInput m_winInput;
        int field_360;
        int field_364;
        int field_368;
    };
}