#include <Glacier/ScriptEngine/AsyncCall_Struct.h>
#include <Glacier/ScriptEngine/FUNCTIONCONTROLLER.h>
#include <Glacier/ScriptEngine/Globals.h>
#include <Glacier/ScriptEngine/LocalVarEntry.h>
#include <Glacier/ScriptEngine/MessageCue.h>
#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/ScriptEngine/ScriptSaveLoad.h>
#include <Glacier/ScriptEngine/ScriptState.h>
#include <Glacier/ScriptEngine/SCRIPTCREATOR.h>
#include <Glacier/ScriptEngine/STATECONTROLLER.h>
#include <Glacier/ZSTL/ISaveMemoryManager.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace Glacier;

namespace
{
    class TestSaveMemoryManager final : public ISaveMemoryManager
    {
    public:
        ~TestSaveMemoryManager()
        {
            for (void* ptr : m_Allocations)
            {
                std::free(ptr);
            }
        }

        void Release(void* ptr)
        {
            FreeMemory(ptr);
        }

    private:
        void* AllocMemory(int lSize) override
        {
            void* ptr = std::malloc(static_cast<size_t>(lSize));
            m_Allocations.push_back(ptr);
            return ptr;
        }

        void FreeMemory(void* ptr) override
        {
            if (!ptr)
            {
                return;
            }

            for (auto it = m_Allocations.begin(); it != m_Allocations.end(); ++it)
            {
                if (*it == ptr)
                {
                    m_Allocations.erase(it);
                    break;
                }
            }

            std::free(ptr);
        }

        std::vector<void*> m_Allocations;
    };

    class ScriptSaveLoadFixture : public testing::Test
    {
    protected:
        void SetUp() override
        {
            g_pSavedPointersMap = nullptr;
            g_pSaveTable = nullptr;
            g_pMessageAllocator = nullptr;
            s_pCurrentSaveGameObject = nullptr;

            g_ScriptAllocator = ZMallocSimple(0x800, 4);
            m_ScriptHeap.fill(0);
            g_ScriptAllocator.AddBlock(m_ScriptHeap.data(), static_cast<uint32_t>(m_ScriptHeap.size()));
        }

        void TearDown() override
        {
            if (g_pSaveTable)
            {
                g_pSaveTable->~SaveTableVector();
                m_SaveMemory.Release(g_pSaveTable);
                g_pSaveTable = nullptr;
            }

            if (g_pSavedPointersMap)
            {
                g_pSavedPointersMap->~SaveTableMap();
                m_SaveMemory.Release(g_pSavedPointersMap);
                g_pSavedPointersMap = nullptr;
            }

            s_pCurrentSaveGameObject = nullptr;
            g_pMessageAllocator = nullptr;
        }

        void InitSaveTables()
        {
            PrepareSave(nullptr);
        }

        template <typename T>
        T* ScriptAlloc()
        {
            return static_cast<T*>(ScriptEngine::Alloc(sizeof(T), __FILE__, __LINE__));
        }

        TestSaveMemoryManager m_SaveMemory;
        std::array<char, 8192> m_ScriptHeap {};
    };
}

TEST_F(ScriptSaveLoadFixture, PrepareSaveCreatesTablesAndSentinelEntries)
{
    PrepareSave(nullptr);

    ASSERT_NE(g_pSavedPointersMap, nullptr);
    ASSERT_NE(g_pSaveTable, nullptr);
    EXPECT_EQ((*g_pSavedPointersMap)[nullptr], 0u);
    ASSERT_EQ(g_pSaveTable->size(), 1u);
    EXPECT_EQ((*g_pSaveTable)[0].m_pAddr, nullptr);
    EXPECT_EQ((*g_pSaveTable)[0].m_lSize, 0u);
}

TEST_F(ScriptSaveLoadFixture, PrepareVariablesAddsFrameAndNestedFrames)
{
    InitSaveTables();

    uint16_t offsets[] = { 0 };
    FUNCTIONCONTROLLER controller { nullptr, 0, 0, nullptr, offsets };
    auto* root = ScriptAlloc<LocalVarEntry>();
    auto* next = ScriptAlloc<LocalVarEntry>();
    auto* prev = ScriptAlloc<LocalVarEntry>();

    root->m_pFunctionController = &controller;
    root->m_pNextVariables = next;
    root->m_pPrevVariables = prev;

    PrepareVariables(root);

    ASSERT_EQ(g_pSaveTable->size(), 4u);
    EXPECT_EQ((*g_pSavedPointersMap)[root], 1u);
    EXPECT_EQ((*g_pSavedPointersMap)[next], 2u);
    EXPECT_EQ((*g_pSavedPointersMap)[prev], 3u);
    EXPECT_EQ((*g_pSaveTable)[1].m_pAddr, root);
    EXPECT_EQ((*g_pSaveTable)[1].m_lSize, sizeof(LocalVarEntry));
    EXPECT_EQ((*g_pSaveTable)[1].m_srt, SRT_VARIABLES);
    EXPECT_EQ((*g_pSaveTable)[1].m_pExtra, offsets);
}

TEST_F(ScriptSaveLoadFixture, PrepareAsyncCallAddsCallChainBeforeLocalVariables)
{
    InitSaveTables();

    auto* first = ScriptAlloc<AsyncCall_Struct>();
    auto* second = ScriptAlloc<AsyncCall_Struct>();
    auto* variables = ScriptAlloc<LocalVarEntry>();

    first->pNext = second;
    first->m_pLVE = variables;

    PrepareAsyncCall(first);

    ASSERT_EQ(g_pSaveTable->size(), 4u);
    EXPECT_EQ((*g_pSavedPointersMap)[first], 1u);
    EXPECT_EQ((*g_pSavedPointersMap)[second], 2u);
    EXPECT_EQ((*g_pSavedPointersMap)[variables], 3u);
    EXPECT_EQ((*g_pSaveTable)[1].m_srt, SRT_ASYNCCALL_STRUCT);
    EXPECT_EQ((*g_pSaveTable)[2].m_srt, SRT_ASYNCCALL_STRUCT);
    EXPECT_EQ((*g_pSaveTable)[3].m_srt, SRT_VARIABLES);
}

TEST_F(ScriptSaveLoadFixture, PrepareMessageCueAddsLastBeforeNextCue)
{
    InitSaveTables();

    auto* root = ScriptAlloc<MessageCue>();
    auto* last = ScriptAlloc<MessageCue>();
    auto* next = ScriptAlloc<MessageCue>();

    root->m_pLast = last;
    root->m_pNext = next;

    PrepareMessageCue(root);

    ASSERT_EQ(g_pSaveTable->size(), 4u);
    EXPECT_EQ((*g_pSavedPointersMap)[root], 1u);
    EXPECT_EQ((*g_pSavedPointersMap)[last], 2u);
    EXPECT_EQ((*g_pSavedPointersMap)[next], 3u);
    EXPECT_EQ((*g_pSaveTable)[1].m_srt, SRT_MESSAGECUE);
    EXPECT_EQ((*g_pSaveTable)[2].m_pAddr, last);
    EXPECT_EQ((*g_pSaveTable)[3].m_pAddr, next);
}

TEST_F(ScriptSaveLoadFixture, PrepareScriptVariablesUsesScriptStateFieldAsMapKey)
{
    InitSaveTables();

    uint16_t offsets[] = { 0 };
    STATECONTROLLER controller { nullptr, nullptr, nullptr, nullptr, nullptr, 1, 1, nullptr, nullptr, offsets };
    SCRIPTCREATOR creator {};
    creator.m_lScriptVariablesSize = 16;
    creator.m_pStateController = &controller;

    ScriptState state {};
    state.m_pCreator = &creator;
    state.m_pScriptVariables = ScriptEngine::Alloc(16, __FILE__, __LINE__);

    PrepareScriptVariables(&state);

    ASSERT_EQ(g_pSaveTable->size(), 2u);
    EXPECT_EQ((*g_pSavedPointersMap)[&state.m_pScriptVariables], 1u);
    EXPECT_EQ((*g_pSaveTable)[1].m_pAddr, state.m_pScriptVariables);
    EXPECT_EQ((*g_pSaveTable)[1].m_lSize, 16u);
    EXPECT_EQ((*g_pSaveTable)[1].m_srt, SRT_SCRIPTVARIABLES);
    EXPECT_EQ((*g_pSaveTable)[1].m_pExtra, &controller);
}

TEST_F(ScriptSaveLoadFixture, PrepareStateVariablesUsesScriptStateFieldAsMapKey)
{
    InitSaveTables();

    uint16_t offsets[] = { 0 };
    STATECONTROLLER controller { nullptr, nullptr, nullptr, nullptr, nullptr, 2, 2, nullptr, nullptr, offsets };
    SCRIPTCREATOR creator {};
    creator.m_lStateVariablesSize = 20;

    ScriptState state {};
    state.m_pCreator = &creator;
    state.m_pStateController = &controller;
    state.m_pStateVariables = ScriptEngine::Alloc(20, __FILE__, __LINE__);

    PrepareStateVariables(&state);

    ASSERT_EQ(g_pSaveTable->size(), 2u);
    EXPECT_EQ((*g_pSavedPointersMap)[&state.m_pStateVariables], 1u);
    EXPECT_EQ((*g_pSaveTable)[1].m_pAddr, state.m_pStateVariables);
    EXPECT_EQ((*g_pSaveTable)[1].m_lSize, 20u);
    EXPECT_EQ((*g_pSaveTable)[1].m_srt, SRT_STATEVARIABLES);
    EXPECT_EQ((*g_pSaveTable)[1].m_pExtra, &controller);
}

TEST_F(ScriptSaveLoadFixture, PrepareDataAddsDynamicStringEntries)
{
    InitSaveTables();

    auto* block = static_cast<char**>(ScriptEngine::Alloc(sizeof(char*), __FILE__, __LINE__));
    const char* text = "save me";
    block[0] = const_cast<char*>(text);

    uint16_t offsets[] = { SRT_VARIABLES, 0, sizeof(char*), 0 };
    PrepareData(offsets, reinterpret_cast<int8_t*>(block));

    ASSERT_EQ(g_pSaveTable->size(), 2u);
    EXPECT_EQ((*g_pSavedPointersMap)[block[0]], 1u);
    EXPECT_EQ((*g_pSaveTable)[1].m_pAddr, text);
    EXPECT_EQ((*g_pSaveTable)[1].m_lSize, std::strlen(text) + 1u);
    EXPECT_EQ((*g_pSaveTable)[1].m_srt, SRT_DYNSTRING);
    EXPECT_EQ((*g_pSaveTable)[1].m_pExtra, nullptr);
}
