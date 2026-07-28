#include <Tests/EngineFixture.h>

#include <Glacier/ZSTL/MYSTR.h>

namespace Tests
{
    namespace
    {
        class DummySysInterface final : public Glacier::ZSysInterface
        {
        public:
            DummySysInterface()
                : ZSysInterface(0)
            {
            }

            void Init() override {}
            void PrintStatus() override {}
            bool WindowDoMessages(void*) override { return false; }
            void CloseDownMain() override {}
            void CloseAllWindows() override {}
            void ReloadDLLs() override {}
            void EditorMessage(int, void*, int) override {}
            Glacier::ZDllBase* AddDll(const char*) override { return nullptr; }
            bool RemoveDll(Glacier::ZDllBase*) override { return false; }
            void ReloadRender() override {}
            void CloseForRestart() override {}
            void InitConfiguration() override {}
            void SetGameName(const char*) override {}
            void SetEngineData(Glacier::ZEngineDataBase* pEngineData) override { m_pEngineData = pEngineData; }
            Glacier::MYSTR ConvertFileName(const char* pName) override { return Glacier::MYSTR(pName ? pName : ""); }
            void RunMain(char*) override {}
            bool RunMainOnce(bool) override { return false; }
            void DumpAutoShots() override {}
            Glacier::TIMETYPE StepTime() override { return Glacier::TIMETYPE(0); }
            void StillFrame() override {}
            void ClearTime() override {}
            void ResetTime() override {}
            void CalcCycSec() override {}
            void Sleep(float) override {}
            int64_t TimeStampCounter(const char*, int) override { return 0; }
        };
    }

    void EngineFixture::SetUp()
    {
        m_PreviousSysInterface = Glacier::g_pSysInterface;
        m_PreviousGlobalCom = Glacier::g_pGlobalCom;
        m_PreviousGlobalCOM = Glacier::g_pGlobalCOM;

        Glacier::g_pGlobalCom = &Glacier::g_GlobalCom;
        Glacier::g_pGlobalCOM = &Glacier::g_GlobalCom;

        m_SysInterface = std::make_unique<DummySysInterface>();
        m_EngineData = std::make_unique<Glacier::ZEngineDataBase>("");
        m_SysInterface->SetEngineData(m_EngineData.get());
    }

    void EngineFixture::TearDown()
    {
        if (m_SysInterface)
            m_SysInterface->SetEngineData(nullptr);

        m_EngineData.reset();
        m_SysInterface.reset();

        Glacier::g_pSysInterface = m_PreviousSysInterface;
        Glacier::g_pGlobalCom = m_PreviousGlobalCom;
        Glacier::g_pGlobalCOM = m_PreviousGlobalCOM;
    }

    Glacier::ZEngineDataBase& EngineFixture::EngineData()
    {
        return *m_EngineData;
    }

    Glacier::ZSysInterface& EngineFixture::SysInterface()
    {
        return *m_SysInterface;
    }
}
