#pragma once

#include <Glacier/Com/CGlobalCom.h>
#include <Glacier/Com/Globals.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <gtest/gtest.h>

#include <memory>

namespace Tests
{
    class EngineFixture : public testing::Test
    {
    protected:
        void SetUp() override;
        void TearDown() override;

        Glacier::ZEngineDataBase& EngineData();
        Glacier::ZSysInterface& SysInterface();

    private:
        std::unique_ptr<Glacier::ZSysInterface> m_SysInterface;
        std::unique_ptr<Glacier::ZEngineDataBase> m_EngineData;
        Glacier::ZSysInterface* m_PreviousSysInterface{};
        Glacier::CSharedCom* m_PreviousGlobalCom{};
        Glacier::CGlobalCom* m_PreviousGlobalCOM{};
    };
}
