#pragma once

namespace Glacier
{
    class ZActionMapTree
    {
    public:
        // vftable
        virtual void Release();

        // data
        class ZActionMapTree* m_parent; //+0x4
        class ZActionMapTree* m_left;   //+0x8
        class ZActionMapTree* m_right;  //+0xC
        int field_10;                   //+0x10
        int field_14;                   //+0x14
        int field_18;                   //+0x18
        char m_name[0x24];              //+0x1C
    };

    class ZActionManager
    {
    public:
        // vftable
        virtual void Release(bool);
        virtual void SetJoinControllers(bool, int);
        virtual void* GetJoinControllers();
        virtual void AddBindings(const char* bindingsStr);
        virtual void LoadBindings(const char* bindingsStr);
        virtual const char* GetKeyName(const char* key);
        virtual const char* GetSystemKey(const char* key, bool a2);
        virtual int* GetDisableInputTimers(int*);
        virtual void DisableInputTimer(float);
        virtual void RevertToDefault();
        virtual void SetDebugKeys(bool);
        virtual bool GetDebugKeys();
        virtual void Enable();
        virtual void Disable();
        virtual bool IsEnabled();

        // data
        ZActionMapTree m_root;
        ZActionMapTree* m_lastNode;
        int  field_48;
        bool m_debugKeys;
        bool m_isEnabled;
        char field_4E;
        char field_4F;
        int  field_50;
    };
}