#include <Tests/EngineFixture.h>

#include <Glacier/IK/ZActionDispatcher.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/IK/ZLnkAction.h>
#include <Glacier/IK/ZLnkActionQueue.h>
#include <Glacier/Serializer/ZPackedInput.h>
#include <Glacier/Serializer/ZPackedOutput.h>
#include <Glacier/ZUniMemory.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace Glacier;

namespace
{
    struct ActionStats
    {
        static int Live;
        static int Terminated;
    };
    int ActionStats::Live = 0;
    int ActionStats::Terminated = 0;

    class TestAction final : public ZLnkAction
    {
    public:
        explicit TestAction(uint32_t lActionId, bool bDeleteOnExit = true)
            : ZLnkAction(lActionId)
            , m_bDeleteOnExit(bDeleteOnExit)
        {
            ++ActionStats::Live;
            Live().push_back(this);
        }

        ~TestAction() override
        {
            --ActionStats::Live;
            auto& live = Live();
            live.erase(std::remove(live.begin(), live.end(), this), live.end());
        }

        static std::vector<TestAction*>& Live()
        {
            static std::vector<TestAction*> s_Live;
            return s_Live;
        }

        // ZLnkAction overrides
        bool Execute() override { ++m_ExecuteCount; return m_bExecuteResult; }
        bool CallBack() override { ++m_CallBackCount; return m_bCallBackResult; }
        bool Update() override { ++m_UpdateCount; return m_bUpdateResult; }
        void Terminate() override { ++m_TerminateCount; ++ActionStats::Terminated; }
        bool DeleteOnExit() const override { return m_bDeleteOnExit; }
        bool AnimEnd(Animation::ActiveAnimation*) override { return m_bAnimEndResult; }
        bool OnMetaKey(Animation::ActiveAnimation*, Animation::ZMetaKey*, const char*) override { return m_bOnMetaKeyResult; }
        bool SupportsLoadSave() override { return m_bSupportsLoadSave; }
        void LoadSave(ISerializerStream& stream, bool bSaving) override
        {
            ZLnkAction::LoadSave(stream, bSaving);
            stream.Exchange("TestAction.m_magic", m_magic);
        }

        // behavior flags
        bool m_bExecuteResult{ false };
        bool m_bCallBackResult{ true };
        bool m_bUpdateResult{ true };
        bool m_bAnimEndResult{ false };
        bool m_bOnMetaKeyResult{ true };
        bool m_bSupportsLoadSave{ true };

        // per-instance counters
        int m_ExecuteCount{ 0 };
        int m_CallBackCount{ 0 };
        int m_UpdateCount{ 0 };
        int m_TerminateCount{ 0 };

        int32_t m_magic{ 0 };

    private:
        bool m_bDeleteOnExit;
    };

    // ZIKLNKOBJ cannot be constructed in unit tests (its ctor needs the material
    // description DB and a base geom), but the dispatcher/queue only use two of its
    // pieces through a ZIKLNKOBJ*:
    //   - the CreateLnkAction virtual (vtable slot)
    //   - ZIKLNKOBJ::m_Active (a raw uint32 at byte offset 0xF4)
    // FakeActionActor provides exactly those two pieces.
    struct TestActionFactory
    {
        ZLnkAction* CreateLnkAction(uint32_t lActionId);
    };

    ZLnkAction* TestActionFactory::CreateLnkAction(uint32_t lActionId)
    {
        return ZUniMemory::New<TestAction>(lActionId, false);
    }

    struct FakeActorStorage
    {
        alignas(4) uint8_t m_Padding[0xF4];
        uint32_t m_Active;
    };

    class FakeActionActor
    {
    public:
        FakeActionActor()
        {
            using FactoryMfp = ZLnkAction* (TestActionFactory::*)(uint32_t);
            const FactoryMfp factoryMfp = &TestActionFactory::CreateLnkAction;

            uintptr_t factoryAddr = 0;
            static_assert(sizeof(factoryMfp) >= sizeof(void*));
            std::memcpy(&factoryAddr, &factoryMfp, sizeof(void*));

            // The dispatcher/queue only ever call the CreateLnkAction virtual on the
            // actor. Fill every vtable slot with the factory so that call resolves to
            // it no matter which slot index ZIKLNKOBJ assigns to CreateLnkAction.
            m_VTable.assign(2048, factoryAddr);

            m_Storage = std::make_unique<FakeActorStorage>();
            *reinterpret_cast<uintptr_t*>(m_Storage.get()) = reinterpret_cast<uintptr_t>(m_VTable.data());

            // Layout sanity check: ZIKLNKOBJ::m_Active must sit at byte offset 0xF4.
            FakeActorStorage probe{};
            const size_t activeOffset = reinterpret_cast<const char*>(&probe.m_Active) - reinterpret_cast<const char*>(&probe);
            ZASSERT(activeOffset == 0xF4);
        }

        ZIKLNKOBJ* Actor()
        {
            return reinterpret_cast<ZIKLNKOBJ*>(m_Storage.get());
        }

        uint32_t& Active()
        {
            return m_Storage->m_Active;
        }

    private:
        std::vector<uintptr_t> m_VTable;
        std::unique_ptr<FakeActorStorage> m_Storage;
    };

    struct MemoryOutputStream final : public IOutputStream
    {
        std::vector<char> Bytes;

        uint32_t Write(const void* pAddr, const uint32_t lSize) override
        {
            const auto* bytes = static_cast<const char*>(pAddr);
            Bytes.insert(Bytes.end(), bytes, bytes + lSize);
            return lSize;
        }
    };

    struct MemoryInputStream final : public ZInputStreamBase
    {
        const std::vector<char>& Bytes;
        size_t Offset{ 0 };

        explicit MemoryInputStream(const std::vector<char>& bytes)
            : Bytes(bytes)
        {
            m_ChangeEndianness = false;
        }

        uint32_t ReadRaw(char* address, const uint32_t size) override
        {
            const uint32_t remaining = static_cast<uint32_t>(Bytes.size() - Offset);
            const uint32_t readSize = std::min(size, remaining);

            if (readSize != 0)
                std::memcpy(address, Bytes.data() + Offset, readSize);

            Offset += readSize;
            return readSize;
        }

        uint32_t ReadChangeEndianness(char* address, const uint32_t size, const uint32_t mask) override
        {
            uint32_t readSize = 0;
            while (readSize < size && Offset < Bytes.size())
            {
                address[readSize ^ mask] = Bytes[Offset++];
                ++readSize;
            }

            return readSize;
        }
    };

    class TestDispatcher : public ZActionDispatcher
    {
    public:
        using ZActionDispatcher::ZActionDispatcher;
        using ZActionDispatcher::LoadSaveLnkAction;
    };

    class LnkActionTest : public Tests::EngineFixture
    {
    protected:
        void SetUp() override
        {
            Tests::EngineFixture::SetUp();
            ActionStats::Live = 0;
            ActionStats::Terminated = 0;
            m_Actor = std::make_unique<FakeActionActor>();
        }

        void TearDown() override
        {
            m_Actor.reset();

            while (!TestAction::Live().empty())
                ZUniMemory::Delete(TestAction::Live().back());

            Tests::EngineFixture::TearDown();
        }

        ZIKLNKOBJ* Actor()
        {
            return m_Actor->Actor();
        }

        uint32_t& ActorActive()
        {
            return m_Actor->Active();
        }

        std::unique_ptr<FakeActionActor> m_Actor;
    };

    // ---- ZLnkAction ----

    TEST_F(LnkActionTest, ZLnkActionGetActionIdReturnsCtorId)
    {
        TestAction action(0x1234);
        EXPECT_EQ(action.GetActionId(), 0x1234u);
    }

    TEST_F(LnkActionTest, ZLnkActionDefaultsMatchBaseImplementation)
    {
        ZLnkAction action(1);
        EXPECT_FALSE(action.Execute());
        EXPECT_TRUE(action.CallBack());
        EXPECT_TRUE(action.Update());
        EXPECT_TRUE(action.DeleteOnExit());
        EXPECT_FALSE(action.SupportsLoadSave());
        EXPECT_FALSE(action.AnimEnd(nullptr));
        EXPECT_TRUE(action.OnMetaKey(nullptr, nullptr, nullptr));
        EXPECT_STREQ(action.Name(), "N/A");
        action.Terminate();
        action.Save(nullptr);
        action.Load(nullptr);
    }

    TEST_F(LnkActionTest, ZLnkActionLoadSaveRoundTripsActionId)
    {
        MemoryOutputStream memory;
        {
            ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_TagsAndDictionary, ISerializerStream::CONTENT_SimpleRepack, false);
            TestAction action(0xCAFE);
            action.m_magic = 42;
            action.LoadSave(output, true);
            output.End();
        }

        MemoryInputStream inputStream(memory.Bytes);
        ZPackedInput input(&inputStream);
        TestAction action(0);
        action.LoadSave(input, false);
        input.End();

        EXPECT_EQ(action.GetActionId(), 0xCAFEu);
        EXPECT_EQ(action.m_magic, 42);
    }

    // ---- ZActionDispatcher ----

    TEST_F(LnkActionTest, DispatcherExecuteStoresCurrentActionOnSuccess)
    {
        ZActionDispatcher dispatcher(nullptr);
        auto* action = ZUniMemory::New<TestAction>(1, false);
        action->m_bExecuteResult = true;

        ASSERT_TRUE(dispatcher.Execute(action));
        EXPECT_EQ(dispatcher.m_pCurrentAction, action);
        EXPECT_EQ(action->m_ExecuteCount, 1);
    }

    TEST_F(LnkActionTest, DispatcherExecuteRemovesActionWhenExecuteFails)
    {
        ZActionDispatcher dispatcher(nullptr);
        auto* action = ZUniMemory::New<TestAction>(1, false);
        action->m_bExecuteResult = false;

        ASSERT_FALSE(dispatcher.Execute(action));
        EXPECT_EQ(dispatcher.m_pCurrentAction, nullptr);
        EXPECT_EQ(action->m_TerminateCount, 1);
        EXPECT_EQ(ActionStats::Live, 1);
    }

    TEST_F(LnkActionTest, DispatcherExecuteThrowsWhenAlreadyHasAction)
    {
        ZActionDispatcher dispatcher(nullptr);
        auto* first = ZUniMemory::New<TestAction>(1, false);
        first->m_bExecuteResult = true;
        auto* second = ZUniMemory::New<TestAction>(2, false);

        ASSERT_TRUE(dispatcher.Execute(first));
        EXPECT_THROW(dispatcher.Execute(second), std::runtime_error);
        EXPECT_EQ(dispatcher.m_pCurrentAction, first);
    }

    TEST_F(LnkActionTest, DispatcherUpdateRemovesActionWhenUpdateReturnsFalse)
    {
        ZActionDispatcher dispatcher(nullptr);
        auto* action = ZUniMemory::New<TestAction>(1, false);
        action->m_bExecuteResult = true;
        ASSERT_TRUE(dispatcher.Execute(action));

        action->m_bUpdateResult = false;
        dispatcher.Update();

        EXPECT_EQ(dispatcher.m_pCurrentAction, nullptr);
        EXPECT_EQ(action->m_UpdateCount, 1);
        EXPECT_EQ(action->m_TerminateCount, 1);
    }

    TEST_F(LnkActionTest, DispatcherUpdateKeepsActionWhileUpdateReturnsTrue)
    {
        ZActionDispatcher dispatcher(nullptr);
        auto* action = ZUniMemory::New<TestAction>(1, false);
        action->m_bExecuteResult = true;
        ASSERT_TRUE(dispatcher.Execute(action));

        dispatcher.Update();

        EXPECT_EQ(dispatcher.m_pCurrentAction, action);
        EXPECT_EQ(action->m_UpdateCount, 1);
        EXPECT_EQ(action->m_TerminateCount, 0);
    }

    TEST_F(LnkActionTest, DispatcherCallBackRemovesActionWhenCallBackReturnsFalse)
    {
        ZActionDispatcher dispatcher(nullptr);
        auto* action = ZUniMemory::New<TestAction>(1, false);
        action->m_bExecuteResult = true;
        ASSERT_TRUE(dispatcher.Execute(action));

        action->m_bCallBackResult = false;
        dispatcher.CallBack();

        EXPECT_EQ(dispatcher.m_pCurrentAction, nullptr);
        EXPECT_EQ(action->m_TerminateCount, 1);
    }

    TEST_F(LnkActionTest, DispatcherOnMetaKeyRemovesActionWhenItReturnsFalse)
    {
        ZActionDispatcher dispatcher(nullptr);
        auto* action = ZUniMemory::New<TestAction>(1, false);
        action->m_bExecuteResult = true;
        ASSERT_TRUE(dispatcher.Execute(action));

        action->m_bOnMetaKeyResult = false;
        dispatcher.OnMetaKey(nullptr, nullptr, nullptr);

        EXPECT_EQ(dispatcher.m_pCurrentAction, nullptr);
        EXPECT_EQ(action->m_TerminateCount, 1);
    }

    TEST_F(LnkActionTest, DispatcherAnimEndRemovesActionWhenItReturnsTrue)
    {
        ZActionDispatcher dispatcher(nullptr);
        auto* action = ZUniMemory::New<TestAction>(1, false);
        action->m_bExecuteResult = true;
        ASSERT_TRUE(dispatcher.Execute(action));

        action->m_bAnimEndResult = true;
        dispatcher.AnimEnd(nullptr);

        EXPECT_EQ(dispatcher.m_pCurrentAction, nullptr);
        EXPECT_EQ(action->m_TerminateCount, 1);
    }

    TEST_F(LnkActionTest, DispatcherAnimEndKeepsActionWhenItReturnsFalse)
    {
        ZActionDispatcher dispatcher(nullptr);
        auto* action = ZUniMemory::New<TestAction>(1, false);
        action->m_bExecuteResult = true;
        ASSERT_TRUE(dispatcher.Execute(action));

        dispatcher.AnimEnd(nullptr);

        EXPECT_EQ(dispatcher.m_pCurrentAction, action);
        EXPECT_EQ(action->m_TerminateCount, 0);
    }

    TEST_F(LnkActionTest, DispatcherDropActionRemovesCurrentAction)
    {
        ZActionDispatcher dispatcher(nullptr);
        auto* action = ZUniMemory::New<TestAction>(1, false);
        action->m_bExecuteResult = true;
        ASSERT_TRUE(dispatcher.Execute(action));

        dispatcher.DropAction();

        EXPECT_EQ(dispatcher.m_pCurrentAction, nullptr);
        EXPECT_EQ(action->m_TerminateCount, 1);
    }

    TEST_F(LnkActionTest, DispatcherRemoveCurrentActionTerminatesAction)
    {
        ZActionDispatcher dispatcher(nullptr);
        auto* action = ZUniMemory::New<TestAction>(1, false);
        action->m_bExecuteResult = true;
        ASSERT_TRUE(dispatcher.Execute(action));

        dispatcher.RemoveCurrentAction();

        EXPECT_EQ(dispatcher.m_pCurrentAction, nullptr);
        EXPECT_EQ(action->m_TerminateCount, 1);
    }

    TEST_F(LnkActionTest, DispatcherDestructorDeletesCurrentActionWhenDeleteOnExit)
    {
        {
            ZActionDispatcher dispatcher(nullptr);
            auto* action = ZUniMemory::New<TestAction>(1, true);
            action->m_bExecuteResult = true;
            ASSERT_TRUE(dispatcher.Execute(action));
            EXPECT_EQ(ActionStats::Live, 1);
        }

        EXPECT_EQ(ActionStats::Live, 0);
    }

    TEST_F(LnkActionTest, DispatcherLoadSaveRoundTripsCurrentAction)
    {
        MemoryOutputStream memory;
        {
            ZActionDispatcher dispatcher(Actor());
            auto* action = ZUniMemory::New<TestAction>(0x55, false);
            action->m_bExecuteResult = true;
            action->m_magic = 0x1122;
            ASSERT_TRUE(dispatcher.Execute(action));

            ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_TagsAndDictionary, ISerializerStream::CONTENT_SimpleRepack, false);
            dispatcher.LoadSave(output, true);
            output.End();
        }

        MemoryInputStream inputStream(memory.Bytes);
        ZPackedInput input(&inputStream);
        {
            ZActionDispatcher dispatcher(Actor());
            dispatcher.LoadSave(input, false);
            input.End();

            ASSERT_NE(dispatcher.m_pCurrentAction, nullptr);
            EXPECT_EQ(dispatcher.m_pCurrentAction->GetActionId(), 0x55u);
            EXPECT_EQ(static_cast<TestAction*>(dispatcher.m_pCurrentAction)->m_magic, 0x1122);
        }
    }

    TEST_F(LnkActionTest, DispatcherLoadSaveSkipsUnsupportedActionWhenSaving)
    {
        MemoryOutputStream memory;
        {
            ZActionDispatcher dispatcher(nullptr);
            auto* action = ZUniMemory::New<TestAction>(1, false);
            action->m_bExecuteResult = true;
            action->m_bSupportsLoadSave = false;
            ASSERT_TRUE(dispatcher.Execute(action));

            ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_TagsAndDictionary, ISerializerStream::CONTENT_SimpleRepack, false);
            dispatcher.LoadSave(output, true);
            output.End();
        }

        MemoryInputStream inputStream(memory.Bytes);
        ZPackedInput input(&inputStream);
        {
            ZActionDispatcher dispatcher(nullptr);
            dispatcher.LoadSave(input, false);
            input.End();

            EXPECT_EQ(dispatcher.m_pCurrentAction, nullptr);
        }
    }

    TEST_F(LnkActionTest, LoadSaveLnkActionCreatesActionByIdThroughActor)
    {
        TestDispatcher dispatcher(Actor());

        MemoryOutputStream memory;
        {
            ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_TagsAndDictionary, ISerializerStream::CONTENT_SimpleRepack, false);
            TestAction saveAction(0x77);
            saveAction.m_magic = 5;
            ZLnkAction* pSaveAction = &saveAction;
            dispatcher.LoadSaveLnkAction(output, true, &pSaveAction);
            output.End();
        }

        MemoryInputStream inputStream(memory.Bytes);
        ZPackedInput input(&inputStream);
        ZLnkAction* pAction = nullptr;
        dispatcher.LoadSaveLnkAction(input, false, &pAction);
        input.End();

        ASSERT_NE(pAction, nullptr);
        EXPECT_EQ(pAction->GetActionId(), 0x77u);
        EXPECT_EQ(static_cast<TestAction*>(pAction)->m_magic, 5);
    }

    // ---- ZLnkActionQueue ----

    TEST_F(LnkActionTest, QueueExecuteAddsActionToQueue)
    {
        ZLnkActionQueue queue(Actor());
        auto* action = ZUniMemory::New<TestAction>(1);
        ASSERT_TRUE(queue.Execute(action));

        ASSERT_NE(queue.m_pActionQueue, nullptr);
        EXPECT_EQ(queue.m_pActionQueue->Count(), 1);
        EXPECT_TRUE(queue.CheckActionQueueForId(1));
        EXPECT_FALSE(queue.CheckActionQueueForId(2));
    }

    TEST_F(LnkActionTest, QueueExecuteAssertsOnNullAction)
    {
        ZLnkActionQueue queue(Actor());
        EXPECT_THROW(queue.Execute(nullptr), std::runtime_error);
    }

    TEST_F(LnkActionTest, QueueUpdateDispatchesFirstActionAndSetsActiveBit)
    {
        ZLnkActionQueue queue(Actor());
        auto* first = ZUniMemory::New<TestAction>(0x100);
        first->m_bExecuteResult = true;
        auto* second = ZUniMemory::New<TestAction>(0x200);
        ASSERT_TRUE(queue.Execute(first));
        ASSERT_TRUE(queue.Execute(second));

        queue.Update();

        EXPECT_EQ(queue.m_pCurrentAction, first);
        EXPECT_EQ(ActorActive(), 0x100u);
        EXPECT_EQ(first->m_ExecuteCount, 1);
    }

    TEST_F(LnkActionTest, QueueUpdateDoesNothingWhenQueueEmpty)
    {
        ZLnkActionQueue queue(Actor());
        queue.Update();

        EXPECT_EQ(queue.m_pCurrentAction, nullptr);
        EXPECT_EQ(ActorActive(), 0u);
    }

    TEST_F(LnkActionTest, QueueUpdateRemovesFailedActionFromQueue)
    {
        ZLnkActionQueue queue(Actor());
        auto* action = ZUniMemory::New<TestAction>(1, false);
        action->m_bExecuteResult = false;
        ASSERT_TRUE(queue.Execute(action));

        queue.Update();

        EXPECT_EQ(queue.m_pCurrentAction, nullptr);
        EXPECT_EQ(queue.m_pActionQueue->Count(), 0);
        EXPECT_EQ(ActorActive(), 0u);
        EXPECT_EQ(ActionStats::Live, 0);
        EXPECT_EQ(ActionStats::Terminated, 1);
    }

    TEST_F(LnkActionTest, QueueDispatchNextActionStartsActionAndReturnsTrue)
    {
        ZLnkActionQueue queue(Actor());
        auto* action = ZUniMemory::New<TestAction>(0x50);
        action->m_bExecuteResult = true;
        ASSERT_TRUE(queue.Execute(action));

        ASSERT_TRUE(queue.DispatchNextAction());
        EXPECT_EQ(queue.m_pCurrentAction, action);
        EXPECT_EQ(ActorActive(), 0x50u);
        EXPECT_EQ(action->m_ExecuteCount, 1);
    }

    TEST_F(LnkActionTest, QueueDispatchNextActionReturnsFalseWhenQueueEmpty)
    {
        ZLnkActionQueue queue(Actor());
        EXPECT_FALSE(queue.DispatchNextAction());
        EXPECT_EQ(queue.m_pCurrentAction, nullptr);
        EXPECT_EQ(ActorActive(), 0u);
    }

    TEST_F(LnkActionTest, QueueRemoveCurrentActionClearsBitAndQueue)
    {
        ZLnkActionQueue queue(Actor());
        auto* first = ZUniMemory::New<TestAction>(0x10);
        first->m_bExecuteResult = true;
        auto* second = ZUniMemory::New<TestAction>(0x20);
        ASSERT_TRUE(queue.Execute(first));
        ASSERT_TRUE(queue.Execute(second));

        ASSERT_TRUE(queue.DispatchNextAction());
        ASSERT_EQ(queue.m_pCurrentAction, first);
        ASSERT_EQ(ActorActive(), 0x10u);

        queue.RemoveCurrentAction();

        EXPECT_EQ(queue.m_pCurrentAction, nullptr);
        EXPECT_EQ(queue.m_pActionQueue->Count(), 1);
        EXPECT_EQ(ActorActive(), 0u);
        EXPECT_EQ(ActionStats::Terminated, 1);
        EXPECT_FALSE(queue.CheckActionQueueForId(0x10));
        EXPECT_TRUE(queue.CheckActionQueueForId(0x20));
    }

    TEST_F(LnkActionTest, QueueCheckActionQueueForIdScansAllActions)
    {
        ZLnkActionQueue queue(Actor());
        ASSERT_TRUE(queue.Execute(ZUniMemory::New<TestAction>(0x100)));
        ASSERT_TRUE(queue.Execute(ZUniMemory::New<TestAction>(0x200)));
        ASSERT_TRUE(queue.Execute(ZUniMemory::New<TestAction>(0x300)));

        EXPECT_TRUE(queue.CheckActionQueueForId(0x100));
        EXPECT_TRUE(queue.CheckActionQueueForId(0x300));
        EXPECT_FALSE(queue.CheckActionQueueForId(0x400));
        EXPECT_FALSE(queue.CheckActionQueueForId(0));
    }

    TEST_F(LnkActionTest, QueueCheckActionQueueForIdReturnsFalseWhenEmpty)
    {
        ZLnkActionQueue queue(Actor());
        EXPECT_FALSE(queue.CheckActionQueueForId(1));
    }

    TEST_F(LnkActionTest, QueueLoadSaveRoundTripsActionsAndCurrentAction)
    {
        MemoryOutputStream memory;
        {
            ZLnkActionQueue queue(Actor());
            auto* first = ZUniMemory::New<TestAction>(0xAA);
            first->m_bExecuteResult = true;
            first->m_magic = 11;
            auto* second = ZUniMemory::New<TestAction>(0xBB);
            second->m_magic = 22;
            ASSERT_TRUE(queue.Execute(first));
            ASSERT_TRUE(queue.Execute(second));
            ASSERT_TRUE(queue.DispatchNextAction());

            ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_TagsAndDictionary, ISerializerStream::CONTENT_SimpleRepack, false);
            queue.LoadSave(output, true);
            output.End();
        }

        MemoryInputStream inputStream(memory.Bytes);
        ZPackedInput input(&inputStream);
        {
            ZLnkActionQueue queue(Actor());
            queue.LoadSave(input, false);
            input.End();

            ASSERT_NE(queue.m_pActionQueue, nullptr);
            EXPECT_EQ(queue.m_pActionQueue->Count(), 2);

            ASSERT_NE(queue.m_pCurrentAction, nullptr);
            EXPECT_EQ(queue.m_pCurrentAction->GetActionId(), 0xAAu);
            EXPECT_EQ(static_cast<TestAction*>(queue.m_pCurrentAction)->m_magic, 11);

            std::vector<uint32_t> ids;
            for (auto* pAction : queue.m_pActionQueue->As<ZLnkAction*>())
                ids.push_back(pAction->GetActionId());

            EXPECT_EQ(ids, std::vector<uint32_t>({ 0xAA, 0xBB }));
        }
    }

    TEST_F(LnkActionTest, QueueLoadSaveRoundTripsEmptyQueue)
    {
        MemoryOutputStream memory;
        {
            ZLnkActionQueue queue(Actor());
            ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_TagsAndDictionary, ISerializerStream::CONTENT_SimpleRepack, false);
            queue.LoadSave(output, true);
            output.End();
        }

        MemoryInputStream inputStream(memory.Bytes);
        ZPackedInput input(&inputStream);
        {
            ZLnkActionQueue queue(Actor());
            queue.LoadSave(input, false);
            input.End();

            EXPECT_EQ(queue.m_pCurrentAction, nullptr);
            EXPECT_EQ(queue.m_pActionQueue, nullptr);
        }
    }

    TEST_F(LnkActionTest, QueueLoadSaveThrowsWhenActionDoesNotSupportLoadSave)
    {
        ZLnkActionQueue queue(Actor());
        auto* action = ZUniMemory::New<TestAction>(1);
        action->m_bSupportsLoadSave = false;
        ASSERT_TRUE(queue.Execute(action));

        MemoryOutputStream memory;
        ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_TagsAndDictionary, ISerializerStream::CONTENT_SimpleRepack, false);
        EXPECT_THROW(queue.LoadSave(output, true), std::runtime_error);
        output.End();
    }
}
