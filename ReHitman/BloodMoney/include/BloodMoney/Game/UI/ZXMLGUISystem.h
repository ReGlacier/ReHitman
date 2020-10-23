#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZWinEvents.h>
#include <Glacier/ZMath.h>
#include <Glacier/ZRTTI.h>

#include <BloodMoney/Game/UI/UIFWD.h>

namespace Hitman::BloodMoney
{
    class ZXMLGUISystem
    {
    public:
        /// === members ===
        char pad_0014[16]; //0x0014
        ZWINDOW* m_window; //0x0024
        char pad_0028[4]; //0x0028
        Glacier::ZRTTI* m_RTTI; //0x002C
        int m_unkInt0; //0x0030
        Glacier::ZGROUP* m_windowGroups[18]; //0x0034

        /// === vftable ===
        virtual void Release(bool); //#0
        virtual void PreSave(int&); //#1
        virtual void PostSave(int&); //#2
        virtual void PreLoad(int&); //#3
        virtual void PostLoad(int&); //#4
        virtual void PostProcess(unsigned int, unsigned int); //#5
        virtual void LoadSave(Glacier::ISerializerStream &,bool); //#6
        virtual void LoadObject(Glacier::IInputSerializerStream &); //#7
        virtual void SaveObject(Glacier::IOutputSerializerStream &); //#8
        virtual void ExchangeObject(Glacier::ISerializerStream &); //#9
        virtual void SetToDefault(); //#10
        virtual int GetTypeID(); //#11
        virtual void *GetProperties(); //#12
        virtual int GetEventPriority(); //#13
        virtual void Init(); //#14
        virtual void Init2(); //#15
        virtual void PostInit(); //#16
        virtual void PostInit2(); //#17
        virtual void CopyData(Glacier::ZEventBase *); //#18
        virtual const char* EventName(); //#19
        virtual void ExpandBounds(float *,float *,float *, Glacier::ZBaseGeom *); //#20
        virtual void PreSaveGame(); //#21
        virtual void RegisterInstance(unsigned int); //#22
        virtual void CheckPointSave(Glacier::ZCheckPointBuffer &); //#23
        virtual void CheckPointLoad(Glacier::ZCheckPointBuffer &); //#24
        virtual void Reset(); //#25
        virtual void TimeUpdate(); //#26
        virtual void FrameUpdate(); //#27
        virtual void Command(Glacier::ZMSGID, void *); //#28
        virtual void DoEvent(int,int,void *); //#29
        virtual void End(); //#30
        virtual void EditorCommand(Glacier::ZMSGID, void *); //#31
        virtual void Remove(); //#32
        virtual void SchedUpdate(); //#33
        virtual void InitBaseConRout(Glacier::ZROUTCLASSINFO *); //#34
        virtual void UnknownCommand(Glacier::ZMSGID, void *); //#35
        virtual ZWINDOWS* GetSystem(); //#36
        virtual void WndMessage(Glacier::ZWMEVENT *); //#37
        virtual void OnCommand(Glacier::ZMSGID); //#38
        virtual void OnMouseMove(Glacier::ZVector2*); //#39
        virtual void OnKeyUp(unsigned int); //#40
        virtual void OnKeyDown(unsigned int); //#41
        virtual void OnKeyPress(unsigned int); //#42
        virtual void OnFocusReceived(unsigned int); //#43
        virtual void OnFocusLost(unsigned int); //#44
        virtual void OnClick(float x, float y); //#45
        virtual void OnWindowOpen(unsigned int, bool); //#46
        virtual void OnWindowClose(unsigned int, bool); //#47
        virtual void OnSliderChange(unsigned int, unsigned int); //#48
        virtual void OnFocusChanged(unsigned int, unsigned int); //#49
        virtual void SetFocus(); //#50
        virtual void AddOtherWindowCount(int); //#51
    };
}