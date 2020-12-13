#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZWinEvents.h>

#include <BloodMoney/Game/SMapGroup.h>
#include <BloodMoney/Game/SIconBase.h>

namespace Hitman::BloodMoney
{
    struct SMapText
    {
        int id; //0x0000
        bool field_04; //0x0004
        bool field_05; //0x0005
        bool field_06; //0x0006
        bool field_07; //0x0007
        char str[92]; //0x0008
    }; //Size: 0x0064

    struct SIconInstance
    {
        int32_t ref;         // SRef id (object id)
        bool field_04;       // must be true if you want to replace icon
        char field_5;        // ?
        char field_6;        // ?
        char field_7;        // ?
        int32_t eIconType;   // icon type (enum)
    };

    class CIngameMap
    {
    public:
        //vftable
        virtual void Release(bool); //#0
        virtual void PreSave(Glacier::ISerializerStream &);
        virtual void PostSave(Glacier::ISerializerStream &);
        virtual void PreLoad(Glacier::ISerializerStream &);
        virtual void PostLoad(Glacier::ISerializerStream &);
        virtual void PostProcess(uint32_t,uint32_t);
        virtual void LoadSave(Glacier::ISerializerStream &,bool);
        virtual void LoadObject(Glacier::IInputSerializerStream &);
        virtual void SaveObject(Glacier::IOutputSerializerStream &);
        virtual void ExchangeObject(Glacier::ISerializerStream &);
        virtual void SetToDefault(void);
        virtual Glacier::ZRTTI* GetTypeID(void);
        virtual void* GetProperties(void);
        virtual int GetEventPriority(void);
        virtual void Init(void);
        virtual void Init2(void);
        virtual void PostInit(void);
        virtual void PostInit2(void);
        virtual void CopyData(Glacier::ZEventBase *);
        virtual void EventName(void);
        virtual void ExpandBounds(float *,float *,float *, Glacier::ZBaseGeom *);
        virtual void PreSaveGame(void);
        virtual void RegisterInstance(uint32_t);
        virtual void CheckPointSave(Glacier::ZCheckPointBuffer &);
        virtual void CheckPointLoad(Glacier::ZCheckPointBuffer &);
        virtual void Reset(void);
        virtual void TimeUpdate(void);
        virtual void FrameUpdate(void);
        virtual void Command(Glacier::ZMSGID ,void *);
        virtual void DoEvent(int,int,void *);
        virtual void End(void);
        virtual void EditorCommand(Glacier::ZMSGID,void *);
        virtual void Remove(void);
        virtual void SchedUpdate(void);
        virtual void InitBaseConRout(/*ZROUTCLASSINFO*/void *);
        virtual void UnknownCommand(Glacier::ZMSGID,void *);
        virtual void GetSystem(void);
        virtual int WndMessage(Glacier::ZWMEVENT *);
        virtual void OnCommand(uint16_t);
        virtual void OnMouseMove(Glacier::Vector3*);
        virtual void OnKeyUp(uint16_t);
        virtual void OnKeyDown(uint16_t);
        virtual void OnKeyPress(uint16_t);
        virtual void OnFocusReceived(uint16_t);
        virtual void OnFocusLost(uint16_t);
        virtual void OnClick(float,float);
        virtual void OnWindowOpen(uint16_t,bool);
        virtual void OnWindowClose(uint16_t,bool);
        virtual void OnSliderChange(uint16_t,uint16_t);
        virtual void OnFocusChanged(uint16_t,uint16_t);
        virtual void AddMapGroup(uint16_t,char const*,uint16_t);
        virtual void OpenMap(void);
        virtual void CloseMap(void);
        virtual void AddIcon(SIconInstance*,bool);
        virtual SIconInstance* GetIcon(uint16_t);
        virtual void RemoveIcon(SIconInstance*);
        virtual SMapText* AddText(SMapText const*);
        virtual void RemoveText(SMapText const*);
        virtual void NotifyUpdate(uint16_t);

        // api
    };
}