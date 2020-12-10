#pragma once

#include <Glacier/Glacier.h>

namespace Glacier
{
    class ZLnkAction
    {
    public:
        // vftable
        virtual void Release(bool);
        virtual int GetActionId();
        virtual bool Execute();
        virtual void CallBack();
        virtual void Update();
        virtual void Terminate();
        virtual void Save(int&); // unused
        virtual void Load(int&); // unused
        virtual const char* Name();
        virtual bool DeleteOnExit();
        virtual void AnimEnd(Animation::ActiveAnimation*);
        virtual void OnMetaKey(Animation::ActiveAnimation*, Animation::ZMetaKey*, const char*);
        virtual bool SupportsLoadSave();
        virtual void LoadSave(ZPackedInput* packedInput, bool);

        // data
        int m_actionId;
        int m_field8;
    };
}