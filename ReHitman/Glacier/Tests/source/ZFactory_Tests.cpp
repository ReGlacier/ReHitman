#include <Glacier/Runtime/ZFactory.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <type_traits>

using namespace Glacier;

namespace
{
    struct TestClassInfo
    {
        int Value{};
    };

    struct PlacementCreator
    {
        struct Create
        {
            template <typename TDerived>
            static TDerived* Do(const TestClassInfo& classInfo)
            {
                auto* instance = new TDerived();
                instance->CreatedFromValue = classInfo.Value;
                return instance;
            }
        };
    };

    struct UIntFactoryBase
    {
        DECLARE_FACTORY(TestClassInfo, uint32_t, PlacementCreator);

        virtual ~UIntFactoryBase() = default;

        static ZFactory<UIntFactoryBase>& GetFactory()
        {
            return Factory;
        }

        static ZFactory<UIntFactoryBase> Factory;
        int CreatedFromValue{};
    };

    ZFactory<UIntFactoryBase> UIntFactoryBase::Factory{};

    struct UIntFactoryDerived final : UIntFactoryBase
    {
    };

    struct StringFactoryBase
    {
        DECLARE_FACTORY(TestClassInfo, const char*, PlacementCreator);

        virtual ~StringFactoryBase() = default;

        static ZFactory<StringFactoryBase>& GetFactory()
        {
            return Factory;
        }

        static ZFactory<StringFactoryBase> Factory;
        int CreatedFromValue{};
    };

    ZFactory<StringFactoryBase> StringFactoryBase::Factory{};

    struct StringFactoryDerived final : StringFactoryBase
    {
    };
}

TEST(ZFactory, ProducerRegistersDataAndCreatesByUIntId)
{
    TestClassInfo classInfo{ 42 };

    ZFactoryProducer<UIntFactoryBase, UIntFactoryDerived> producer(0x1234, classInfo);

    auto* found = UIntFactoryBase::GetFactory().Find(0x1234);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->m_ProducerId, 0x1234u);
    EXPECT_EQ(found->Value, 42);

    UIntFactoryBase* created = UIntFactoryBase::GetFactory().Create(0x1234);

    ASSERT_NE(created, nullptr);
    EXPECT_EQ(created->CreatedFromValue, 42);
    EXPECT_NE(dynamic_cast<UIntFactoryDerived*>(created), nullptr);

    delete created;
}

TEST(ZFactory, ReturnsNullForUnknownProducer)
{
    ZFactory<UIntFactoryBase> factory;

    EXPECT_EQ(factory.Find(0xFFFF), nullptr);
    EXPECT_EQ(factory.Create(0xFFFF), nullptr);
}

TEST(ZFactory, HandlesUIntHashBucketCollisions)
{
    ZFactory<UIntFactoryBase> factory;
    TestClassInfo firstInfo{ 1 };
    TestClassInfo secondInfo{ 2 };
    ZFactory<UIntFactoryBase>::ProducerData first(1, &ZFactoryProducer<UIntFactoryBase, UIntFactoryDerived>::Create, firstInfo);
    ZFactory<UIntFactoryBase>::ProducerData second(17, &ZFactoryProducer<UIntFactoryBase, UIntFactoryDerived>::Create, secondInfo);

    ASSERT_TRUE(factory.Add(&first));
    ASSERT_TRUE(factory.Add(&second));

    EXPECT_EQ(factory.Hash(1), 1);
    EXPECT_EQ(factory.Hash(17), 17);
    EXPECT_EQ(factory.Find(1), &first);
    EXPECT_EQ(factory.Find(17), &second);
}

TEST(ZFactory, CreatesByStringIdUsingContentComparison)
{
    ZFactory<StringFactoryBase> factory;
    TestClassInfo classInfo{ 77 };
    ZFactory<StringFactoryBase>::ProducerData producer("TestProducer", &ZFactoryProducer<StringFactoryBase, StringFactoryDerived>::Create, classInfo);
    char equivalentId[] = "TestProducer";

    ASSERT_TRUE(factory.Add(&producer));

    auto* found = factory.Find(equivalentId);
    ASSERT_EQ(found, &producer);
    EXPECT_TRUE(factory.Equals(producer.m_ProducerId, equivalentId));

    StringFactoryBase* created = factory.Create(equivalentId);

    ASSERT_NE(created, nullptr);
    EXPECT_EQ(created->CreatedFromValue, 77);
    EXPECT_NE(dynamic_cast<StringFactoryDerived*>(created), nullptr);

    delete created;
}
