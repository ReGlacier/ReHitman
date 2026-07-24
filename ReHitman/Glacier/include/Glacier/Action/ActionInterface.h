#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZActionMapTree;
    class ZActionManager;

    struct Action
    {
        struct ZHandle
        {
            // methods
            ZHandle(const char* psName);
            ~ZHandle();

            void SetDevice(int iDevice);
            int Override(int* pDeviceId, int* pControlId);
            bool Digital();
            float Analog();
            int ActivatedBy();
            void ClearInputKeys();
            bool CheckMap();

            // members
            const char* m_szName;
            ZActionMapTree* m_pkMap;
            int m_iSeq;
        };

        struct ZStaticBinds
        {
            // static 
            STATIC_CLASS_VAR(ZStaticBinds, ZStaticBinds*, pFirst);

            // methods
            static ZStaticBinds* GetFirst();
            
            ZStaticBinds(const char* psBinds);
            ZStaticBinds* GetNext() const;
            const char* GetBinds() const;

            // members
            ZStaticBinds* pNext;
            const char* szBinds;
        };


        struct Interface
        {
            // vtbl
            virtual ~Interface() = default;
            virtual void SetJoinControllers(bool, int) = 0;
            virtual bool GetJoinControllers() const = 0;
            virtual bool AddBindings(const char* binds) = 0;
            virtual bool LoadBindings(const char* szFilename) = 0;
            virtual const char* GetKeyName(const char* pKey) = 0;
            virtual const char* GetSystemKeyName(const char* szAction, bool firstKey) = 0;
            virtual const TIMETYPE& GetDisableInputTimes() = 0;
            virtual void DisableInputTimer(double deltaTime) = 0;
            virtual void RevertToDefault() = 0;
            virtual void SetDebugKeys(bool bDebugKeys) = 0;
            virtual bool GetDebugKeys() const = 0;
            virtual void Enable() = 0;
            virtual void Disable() = 0;
            virtual bool IsEnabled() const = 0;

            // methods
            Interface() = default;
        };

        static bool Initialize();
        static void Free();
        
        STATIC_CLASS_VAR(Action, ZActionManager*, instance);
    };
}