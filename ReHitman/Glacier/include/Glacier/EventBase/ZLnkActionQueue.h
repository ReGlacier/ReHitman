#pragma once

#include <Glacier/Glacier.h>

namespace Glacier
{
    class ZIKLNKOBJ;
    class LINKREFTAB;

    class ZLnkActionQueue
    {
    public:
        // vftable
        virtual void Release(bool);
        virtual void Execute(ZLnkAction* action);
        virtual void Update();
        virtual void CallBack();
        virtual void OnMetaKey(Animation::ActiveAnimation*, Animation::ZMetaKey*, const char*);
        virtual void DropAction();
        virtual void RemoveCurrentAction();
        virtual void AnimEnd(Animation::ActiveAnimation*);
        virtual void LoadSave(ZPackedInput* packedInput, bool);
        virtual void CheckActionQueueForId(unsigned int);

        // api
        void DispatchNextAction();
        void RemoveAction(ZLnkAction* action);

        // data (size is 0x10)
        ZLnkAction* m_currentAction;
        ZIKLNKOBJ* m_lnkObj;
        LINKREFTAB* m_actionsQueue;
    };
}