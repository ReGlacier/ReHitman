#pragma once


namespace Glacier
{
    class ZDllBase
    {
    public:
        // vtbl
        virtual ~ZDllBase();
        virtual bool CleanupBeforeCloseDown();
        virtual void FreeMem();
        virtual void Init();
        virtual void End();
        virtual void PushScene(const char* pScene);
        virtual void PopScene();

        virtual void AllocSequenceStart();
        virtual void AllocSequenceEnd();
        virtual void InsertStart();
        virtual void InsertEnd();
        virtual void CrashFree();

        virtual void Set(void* pHandle, const char* psName);
        virtual void* GetAddress(const char* pSymName);
        virtual void PrintStatus();
        
        // methods
        ZDllBase();

        // members
        char* m_pName{nullptr}; // +0x4
        void* m_pHandle{nullptr}; // +0x8
    };
}