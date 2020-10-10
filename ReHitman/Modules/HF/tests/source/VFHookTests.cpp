#include <gtest/gtest.h>

#include <HF/HackingFramework.hpp>

class VFHookTest : public testing::Test
{
protected:
};

class SomeCoolClass
{
public:
    virtual int GetA() const { return 42; }
};

struct AnotherHackedClass
{
    int NewGetA() const { return 24; }
};

TEST_F(VFHookTest, CheckSimpleExampleOnHeap)
{
    auto coolClass = new SomeCoolClass();
    ASSERT_EQ(coolClass->GetA(), 42);

    {
        HF::Hook::VFHook<SomeCoolClass> coolClassHook(coolClass, 0, &AnotherHackedClass::NewGetA);
        ASSERT_EQ(coolClass->GetA(), 24);
    }

    ASSERT_EQ(coolClass->GetA(), 42);
    delete coolClass;
}

TEST_F(VFHookTest, UniquePtrWrapperConstructedOverHookVirtualFunction)
{
    auto coolClass = std::make_unique<SomeCoolClass>();
    ASSERT_EQ(coolClass->GetA(), 42);

    {
        auto hook = HF::Hook::HookVirtualFunction<SomeCoolClass, 0>(coolClass.get(), &AnotherHackedClass::NewGetA);
        ASSERT_EQ(coolClass->GetA(), 24);
    }

    ASSERT_EQ(coolClass->GetA(), 42);
}

static int __stdcall GetMagicAValue() {
    return 512;
}

TEST_F(VFHookTest, MoveExecutionToStdCallStaticFunction)
{
    auto coolClass = std::make_unique<SomeCoolClass>();
    ASSERT_EQ(coolClass->GetA(), 42);

    {
        auto hook = HF::Hook::HookVirtualFunction<SomeCoolClass, 0>(coolClass.get(), &GetMagicAValue);
        ASSERT_EQ(coolClass->GetA(), 512);
    }

    ASSERT_EQ(coolClass->GetA(), 42);

    {
        auto hook = HF::Hook::HookVirtualFunction<SomeCoolClass, 0>(coolClass.get(), &GetMagicAValue);
        ASSERT_EQ(coolClass->GetA(), 512);
    }

    ASSERT_EQ(coolClass->GetA(), 42);
}

class Koban
{
private:
    int Health = 100;
    int Armor = 100;

public:
    virtual int GetHealth() const { return Health; }
    virtual int GetArmor() const { return Armor; }
};

struct ReversedKoban
{
    int HP;
    int Armor;
};

static int __stdcall BypassKoban() {
    ReversedKoban* pUnknownClass;
    __asm {
        mov eax, ecx
        add eax, 0x4 ; skip vftable
        mov pUnknownClass, eax
    }
    pUnknownClass->HP = -100;
    pUnknownClass->Armor = 0;
    return 50;
}

TEST_F(VFHookTest, KillKobanTest)
{
    auto don = std::make_unique<Koban>();
    ASSERT_EQ(don->GetArmor(), 100);
    ASSERT_EQ(don->GetHealth(), 100);

    {
        auto hook = HF::Hook::HookVirtualFunction<Koban, 0>(don.get(), &BypassKoban);
        ASSERT_EQ(don->GetHealth(), 50);
        ASSERT_EQ(don->GetArmor(), 0);
    }

    ASSERT_EQ(don->GetHealth(), -100);
    ASSERT_EQ(don->GetArmor(), 0);
}