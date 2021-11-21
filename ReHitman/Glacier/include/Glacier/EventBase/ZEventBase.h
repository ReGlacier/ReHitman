#pragma once

#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZListNodeBase.h>
#include <Glacier/ZScheduledScript.h>

namespace Glacier
{
    class ZEventBase : public ZListNodeBase
    {
    public:
        /// === members ===
        int32_t m_field14; //0x0014
        int32_t m_field18; //0x0018
        int32_t m_field1C; //0x001C
        int32_t m_field20; //0x0020
        Glacier::ZGEOM* m_geom; //0x0024
        Glacier::ZScheduledScript* m_pScheduledScript; //+0x0028
        Glacier::ZRTTI* m_runtimeTypeInfo; //0x002C

        /// === vftable ===
        virtual void Release(bool);                                                     //#00 | +0
        virtual void PreSave(int&);                                                     //#01 | +4
        virtual void PostSave(int&);                                                    //#02 | +8
        virtual void PreLoad(int&);                                                     //#03 | +C
        virtual void PostLoad(int&);                                                    //#04 | +10
        virtual void PostProcess(int, int);                                             //#05 | +14
        virtual void LoadSave(Glacier::ZPackedInput*, bool);                            //#06 | +18
        virtual void LoadObject(int&);                                                  //#07 | +1C
        virtual void SaveObject(int&);                                                  //#08 | +20
        virtual void ExchangeObject(Glacier::ZPackedInput*);                            //#09 | +24
        virtual void SetToDefault();                                                    //#10 | +28
        virtual int GetTypeID();                                                        //#11 | +2C
        virtual void* GetProperties();                                                  //#12 | +30
        virtual int GetEventPriority();                                                 //#13 | +34
        virtual void Init();                                                            //#14 | +38
        virtual void Init2();                                                           //#15 | +3C
        virtual void PostInit();                                                        //#16 | +40
        virtual void PostInit2();                                                       //#17 | +44
        virtual void CopyData(const ZEventBase*);                                       //#18 | +48
        virtual const char* EventName();                                                //#19 | +4C
        virtual void ExpandBounds(float*, float*, float*, Glacier::ZEntityLocator*);    //#20 | +50
        virtual void PreSaveGame();                                                     //#21 | +54
        virtual void RegisterInstance();                                                //#22 | +58
        virtual void CheckPointSave(int&);                                              //#23 | +5C
        virtual void CheckPointLoad(int&);                                              //#24 | +60
        virtual void Reset();                                                           //#25 | +64
        virtual void TimeUpdate();                                                      //#26 | +68
        virtual void FrameUpdate();                                                     //#27 | +6C
        virtual void Command(Glacier::ZMSGID command, Glacier::ZDATA data);             //#28 | +70
        virtual void DoEvent(int, int, void*);                                          //#29 | +74
        virtual void End();                                                             //#30 | +78
        virtual void EditorCommand(Glacier::ZMSGID command, Glacier::ZDATA data);       //#31 | +7C
        virtual void Remove();                                                          //#32 | +80
        virtual void SchedUpdate();                                                     //#33 | +84
        virtual void InitBaseConRout(Glacier::ZROUTCLASSINFO*);                         //#34 | +88
        virtual void UnknownCommand(Glacier::ZMSGID command, Glacier::ZDATA data);      //#35 | +8C
    }; //Size: 0x0030
}