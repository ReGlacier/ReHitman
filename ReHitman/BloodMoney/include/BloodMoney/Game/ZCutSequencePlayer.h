#pragma once

#include <Glacier/GlacierFWD.h>

namespace Hitman::BloodMoney
{
    class ZCutSequencePlayer
    {
    public:
        // vftable
        virtual void Release(bool);
        virtual void PreSave(int&);
        virtual void PostSave(int&);
        virtual void PreLoad(int&);
        virtual void PostLoad(int&);
        virtual void PostProcess(int, int);
        virtual void LoadSave(Glacier::ZPackedInput*);
        virtual void LoadObject(int&);
        virtual void SaveObject(int&);
        virtual void ExchangeObject(Glacier::ZPackedInput*);
        virtual void SetToDefault();
        virtual int GetTypeID(); //deprecated
        virtual int32_t GetProperties();
        virtual int32_t GetEventPriority();
        virtual void Init();
        virtual void Init2();
        virtual void PostInit();
        virtual void PostInit2();
        virtual void CopyData(Glacier::ZEventBase*);
        virtual int32_t EventName();
        virtual void ExpandBounds(float*, float*, float*, Glacier::ZEntityLocator*);
        virtual void PreSaveGame();
        virtual void RegisterInstance();
        virtual void CheckPointSave(int&); //ZCheckPointBuffer&
        virtual void CheckPointLoad(int&); //ZCheckPointBuffer&
        virtual void Reset();
        virtual void TimeUpdate();
        virtual void FrameUpdate();
        virtual void Command(Glacier::ZMSGID command, void* argOrRes);
        virtual void DoEvent(int, int, void*);
        virtual void End();
        virtual void EditorCommand(Glacier::ZMSGID command, void* argOrRes);
        virtual void Remove();
        virtual void SchedUpdate();
        virtual void InitBaseConRout(Glacier::ZROUTCLASSINFO*);
        virtual void UnknownCommand(Glacier::ZMSGID, void*);

        // data
        int32_t field_4;
        int32_t field_8;
        int32_t field_C;
        int32_t field_10;
        int32_t field_14;
        int32_t field_18;
        int32_t field_1C;
        int32_t field_20;
        Glacier::ZLIST* m_ZLIST_24;
        int field_28;
        int field_2C;
        int field_30;
        Glacier::ZMSGID m_MSG_CutEnd;
        Glacier::ZMSGID m_MSG_ReplyWhenReady;
        Glacier::ZMSGID m_MSG_CutSequence_StartScene;
        Glacier::ZMSGID m_MSG_CutSequence_Start;
        int32_t field_3C;
        int32_t field_40;
        int32_t field_44;
        int32_t field_48;
        int32_t field_4C;
        int32_t field_50;
        int32_t field_54;
        bool m_isPlaying;
        char field_59;
        char field_5A;
        char field_5B;
        int32_t field_5C;
        int32_t field_60;
        int32_t m_isWaitingForReady;
        int32_t m_activeSound;
        char field_6C;
        char field_6D;
        char field_6E;
        char field_6F;
        int32_t m_sound;
    };
}