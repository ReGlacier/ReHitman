#include <BloodMoney/Patches/All/FixFuckedLoaderScreens.h>
#include <BloodMoney/Game/LoaderSequence/ZLoader_Sequence_Script_Reader.h>
#include <BloodMoney/Game/LoaderSequence/ZLoader_Sequence_Wintel_D3D.h>
#include <BloodMoney/Game/LoaderSequence/ZLoader_Sequence_Script.h>
#include <Windows.h>
#include <spdlog/spdlog.h>
#include <d3d9.h>
#include <thread>

namespace Hitman::BloodMoney
{
    static LARGE_INTEGER g_qpcLastFrame = { 0 };
    static LARGE_INTEGER g_qpcFreq = { 0 };

    struct FakeLoaderSequence : public ZLoader_Sequence_Wintel_D3D
    {
        // A "brand new" delta timer calculator
        void UpdateLoaderTimer()
        {
            if (g_qpcFreq.QuadPart == 0)
            {
                QueryPerformanceFrequency(&g_qpcFreq);
                QueryPerformanceCounter(&g_qpcLastFrame);
                m_fCurrentTime = 0.0f;
                return;
            }

            LARGE_INTEGER qpcCurrent;
            QueryPerformanceCounter(&qpcCurrent);

            double deltaTime = (double)(qpcCurrent.QuadPart - g_qpcLastFrame.QuadPart) / (double)g_qpcFreq.QuadPart;
            g_qpcLastFrame = qpcCurrent;
            if (deltaTime > 0.1)
            {
                deltaTime = 0.016666;
            }

            m_fCurrentTime += (float)deltaTime;
        }

        void WaitForReady()
        {
            using IsFinished_t = bool(__thiscall*)(ZLoader_Sequence_Script_Reader*, float);
            IsFinished_t IsFinished = (IsFinished_t)0x004679A0;

            while (!IsFinished(pReader, m_fCurrentTime))
            {
                std::this_thread::yield(); 
            }

            g_qpcLastFrame.QuadPart = 0;
        }
    };

    bool FixFuckedLoaderScreens::Apply(const ModPack& modules) 
    {
        if (!BasicPatch::Apply(modules))
        {
            return false;
        }

        if (auto process = modules.process.lock())
        {
            // Replace render global sleep from 10us -> 40us
            process->writeMemory(0x004AEFA0, { 0x6A, 0x28 });

            // Replace shit math to my shit math
            process->fillMemory(0x004AED99, 0x90, 40);
            process->writeMemory(0x004AED99, { 0x89, 0xF9 }); // mov ecx, edi

            m_correctTimeCalcTrampoline = HF::Hook::HookFunction(
                process, 
                0x004AED99 + 2, 
                &FakeLoaderSequence::UpdateLoaderTimer, 
                {
                    HF::X86::PUSH_AD
                }, 
                {
                    HF::X86::POP_AD,
                    // pop esi
                    0x5E,
                    // pop ebx
                    0x5B
                });
            m_correctTimeCalcTrampoline->setup();

            // 3. replace in-game logic from Finish_0 to our (before WaitForSingleObject<INF>)
            // next 16 bytes to NOP
            process->fillMemory(0x004AE270, 0x90, 16);
            // write first 2 bytes to mov ecx, esi
            process->writeMemory(0x004AE270, { 0x89, 0xF1 }); // mov ecx, esi

            // Store jump to our logic
            m_newWaitLoopTrampoline = HF::Hook::HookFunction(process, 0x004AE270 + 2, &FakeLoaderSequence::WaitForReady, {}, {});
            m_newWaitLoopTrampoline->setup();
        }

        return true;
    }
    
    void FixFuckedLoaderScreens::Revert(const ModPack& modules) 
    {
        BasicPatch::Revert(modules);
    }
}