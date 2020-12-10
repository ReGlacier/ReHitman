#pragma once

#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZListNodeBase.h>

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
        int32_t m_field28; //0x0028
        Glacier::ZRTTI* m_runtimeTypeInfo; //0x002C

        /// === vftable ===
        virtual void Release(bool);
        virtual void PreSave(int&);
        virtual void PostSave(int&);
        virtual void PreLoad(int&);
        virtual void PostLoad(int&);
        virtual void PostProcess(int, int);
        virtual void LoadSave(Glacier::ZPackedInput*, bool);
        virtual void LoadObject(int&);
        virtual void SaveObject(int&);
        virtual void ExchangeObject(Glacier::ZPackedInput*);
        virtual void SetToDefault();
        virtual int GetTypeID();
        virtual void* GetProperties();
        virtual int GetEventPriority();
        virtual void Init();
        virtual void Init2();
        virtual void PostInit();
        virtual void PostInit2();
        virtual void CopyData(const ZEventBase*);
        virtual const char* EventName();
        virtual void ExpandBounds(float*, float*, float*, Glacier::ZEntityLocator*);
        virtual void PreSaveGame();
        virtual void RegisterInstance();
        virtual void CheckPointSave(int&);
        virtual void CheckPointLoad(int&);
        virtual void Reset();
        virtual void TimeUpdate();
        virtual void FrameUpdate();
        virtual void Command(Glacier::ZMSGID command, Glacier::ZDATA data);
        virtual void DoEvent(int, int, void*);
        virtual void End();
        virtual void EditorCommand(Glacier::ZMSGID command, Glacier::ZDATA data);
        virtual void Remove();
        virtual void SchedUpdate();
        virtual void InitBaseConRout(Glacier::ZROUTCLASSINFO*);
        virtual void UnknownCommand(Glacier::ZMSGID command, Glacier::ZDATA data);
    }; //Size: 0x0030
}