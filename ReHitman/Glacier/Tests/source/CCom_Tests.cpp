#include <gtest/gtest.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Com/CCOMType.h>
#include <Glacier/Com/CCOMTypeCast.h>
#include <Glacier/Com/CCOMFormat.h>
#include <cstring>

using namespace Glacier;

namespace Glacier { extern CCOMTypeCast CCOMTypeTable[18]; }

/**
 * @brief Initializes CCOMTypeTable with both eComType-matching entries
 *        (for GetTypeCast(CCOMType)) and name-prefix entries (for
 *        GetTypeCast(const char*) used by SetVal/CComRead).
 */
static void InitCCOMTypeTable()
{
    auto& t = CCOMTypeTable;

    // --- eComType-based entries (used by SetVal(name, value, CCOMType)) ---
    t[0].eComType    = CCOM_TYPE_BOOL;
    t[0].eCComFormat = CCOM_FORMAT_BOOL;
    t[0].eLen        = sizeof(bool);

    t[1].eComType    = CCOM_TYPE_CHAR;
    t[1].eCComFormat = CCOM_FORMAT_CHAR;
    t[1].eLen        = sizeof(char);

    t[2].eComType    = CCOM_TYPE_INT32;
    t[2].eCComFormat = CCOM_FORMAT_int32;
    t[2].eLen        = sizeof(int);

    t[3].eComType    = CCOM_TYPE_FLOAT;
    t[3].eCComFormat = CCOM_FORMAT_FLOAT;
    t[3].eLen        = sizeof(float);

    t[4].eComType    = CCOM_TYPE_STRING;
    t[4].eCComFormat = CCOM_FORMAT_CHAR;
    t[4].eLen        = 0;

    t[5].eComType    = CCOM_TYPE_BLOCK;
    t[5].eCComFormat = CCOM_FORMAT_CLASS;
    t[5].eLen        = 0;

    t[6].eComType    = CCOM_TYPE_PTR;
    t[6].eCComFormat = CCOM_FORMAT_int32;
    t[6].eLen        = sizeof(int);

    // --- Name-prefix entries (used by GetTypeCast(const char*) for SetVal) ---
    // Game convention: first char of key name indicates type
    // b* = bool, i* = int32, f* = float, s* = string

    t[10].lLetters    = 'b' << 24;
    t[10].lTypeLen    = 1;
    t[10].eComType    = CCOM_TYPE_BOOL;
    t[10].eCComFormat = CCOM_FORMAT_BOOL;
    t[10].eLen        = sizeof(bool);

    t[11].lLetters    = 'i' << 24;
    t[11].lTypeLen    = 1;
    t[11].eComType    = CCOM_TYPE_INT32;
    t[11].eCComFormat = CCOM_FORMAT_int32;
    t[11].eLen        = sizeof(int);

    t[12].lLetters    = 'f' << 24;
    t[12].lTypeLen    = 1;
    t[12].eComType    = CCOM_TYPE_FLOAT;
    t[12].eCComFormat = CCOM_FORMAT_FLOAT;
    t[12].eLen        = sizeof(float);

    t[13].lLetters    = 's' << 24;
    t[13].lTypeLen    = 1;
    t[13].eComType    = CCOM_TYPE_STRING;
    t[13].eCComFormat = CCOM_FORMAT_CHAR;
    t[13].eLen        = 0;
}

class CComTest : public ::testing::Test
{
protected:
    CComTest()
    {
        InitCCOMTypeTable();
    }

    void SetUp() override
    {
        m_pCom = new CCom();
    }

    void TearDown() override
    {
        delete m_pCom;
        m_pCom = nullptr;
    }

    CCom* m_pCom = nullptr;
};

// ---------------------------------------------------------------------------
// Constructor / Initial State
// ---------------------------------------------------------------------------

TEST_F(CComTest, ConstructorInitializesMembers)
{
    EXPECT_NE(m_pCom->m_pMemory, nullptr);
    EXPECT_EQ(m_pCom->m_pMemory, m_pCom->m_Memory);
    EXPECT_EQ(m_pCom->m_ComDat.Count(), 0);
    EXPECT_EQ(m_pCom->m_OffsetAlloc.GetNrFreeLinks(), 1u);
}

// ---------------------------------------------------------------------------
// AddValOfType + GetpVal (low-level insert/retrieve)
// ---------------------------------------------------------------------------

TEST_F(CComTest, AddValOfType_StoresValue)
{
    const char* name = "testScore";
    int nameLen = static_cast<int>(strlen(name)) + 1;
    int value = 12345;

    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_INT32);
    ASSERT_NE(pType, nullptr);

    m_pCom->AddValOfType(name, nameLen, pType, sizeof(value), reinterpret_cast<const char*>(&value));

    ComValueInfo* pInfo = m_pCom->GetpVal(name, 0);
    ASSERT_NE(pInfo, nullptr);
    EXPECT_EQ(pInfo->lNameLen, nameLen);
    EXPECT_EQ(pInfo->lDataLen, static_cast<int>(sizeof(value)));
    EXPECT_EQ(pInfo->lType, pType);

    const char* pStoredName = reinterpret_cast<const char*>(pInfo + 1);
    EXPECT_STREQ(pStoredName, name);

    const int* pStoredData = reinterpret_cast<const int*>(pStoredName + nameLen);
    EXPECT_EQ(*pStoredData, value);
}

TEST_F(CComTest, AddValOfType_OverwritesExistingByName)
{
    const char* name = "ammoCount";
    int nameLen = static_cast<int>(strlen(name)) + 1;
    int firstVal = 30;
    int secondVal = 99;

    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_INT32);

    m_pCom->AddValOfType(name, nameLen, pType, sizeof(firstVal), reinterpret_cast<const char*>(&firstVal));
    m_pCom->AddValOfType(name, nameLen, pType, sizeof(secondVal), reinterpret_cast<const char*>(&secondVal));

    ComValueInfo* pInfo = m_pCom->GetpVal(name, 0);
    ASSERT_NE(pInfo, nullptr);

    const char* pData = reinterpret_cast<const char*>(pInfo + 1) + pInfo->lNameLen;
    EXPECT_EQ(*reinterpret_cast<const int*>(pData), secondVal);
}

TEST_F(CComTest, GetpVal_ReturnsNullForMissingKey)
{
    EXPECT_EQ(m_pCom->GetpVal("missing", 0), nullptr);
}

TEST_F(CComTest, GetpVal_ReturnsNullForNullName)
{
    EXPECT_EQ(m_pCom->GetpVal(nullptr, 0), nullptr);
}

// ---------------------------------------------------------------------------
// Exists
// ---------------------------------------------------------------------------

TEST_F(CComTest, Exists_ReturnsTrueForExistingKey)
{
    const char* name = "flag";
    int nameLen = static_cast<int>(strlen(name)) + 1;
    bool val = true;

    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_BOOL);
    m_pCom->AddValOfType(name, nameLen, pType, sizeof(val), reinterpret_cast<const char*>(&val));

    EXPECT_TRUE(m_pCom->Exists(name, 0));
}

TEST_F(CComTest, Exists_ReturnsFalseForMissingKey)
{
    EXPECT_FALSE(m_pCom->Exists("noSuchKey", 0));
}

TEST_F(CComTest, Exists_ReturnsFalseForNullName)
{
    EXPECT_FALSE(m_pCom->Exists(nullptr, 0));
}

// ---------------------------------------------------------------------------
// RemoveVal
// ---------------------------------------------------------------------------

TEST_F(CComTest, RemoveVal_RemovesAndReturnsTrue)
{
    const char* name = "gravity";
    int nameLen = static_cast<int>(strlen(name)) + 1;
    float val = 9.8f;

    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_FLOAT);
    m_pCom->AddValOfType(name, nameLen, pType, sizeof(val), reinterpret_cast<const char*>(&val));

    EXPECT_TRUE(m_pCom->Exists(name, 0));
    EXPECT_TRUE(m_pCom->RemoveVal(name, 0));
    EXPECT_FALSE(m_pCom->Exists(name, 0));
    EXPECT_EQ(m_pCom->GetpVal(name, 0), nullptr);
}

TEST_F(CComTest, RemoveVal_ReturnsFalseForMissingKey)
{
    EXPECT_FALSE(m_pCom->RemoveVal("ghost", 0));
}

TEST_F(CComTest, RemoveVal_ReturnsFalseForNullName)
{
    EXPECT_FALSE(m_pCom->RemoveVal(nullptr, 0));
}

TEST_F(CComTest, RemoveVal_HandlesZeroLengthName)
{
    const char* name = "data";
    int nameLen = static_cast<int>(strlen(name)) + 1;

    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_INT32);
    int val = 42;
    m_pCom->AddValOfType(name, nameLen, pType, sizeof(val), reinterpret_cast<const char*>(&val));

    EXPECT_TRUE(m_pCom->RemoveVal(name, 0));
    EXPECT_FALSE(m_pCom->Exists(name, 0));
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

TEST_F(CComTest, Clear_RemovesAllEntries)
{
    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_INT32);
    int val = 1;
    m_pCom->AddValOfType("a", 2, pType, sizeof(val), reinterpret_cast<const char*>(&val));
    m_pCom->AddValOfType("b", 2, pType, sizeof(val), reinterpret_cast<const char*>(&val));
    m_pCom->AddValOfType("c", 2, pType, sizeof(val), reinterpret_cast<const char*>(&val));

    EXPECT_EQ(m_pCom->m_ComDat.Count(), 3);

    m_pCom->Clear();

    EXPECT_EQ(m_pCom->m_ComDat.Count(), 0);
    EXPECT_FALSE(m_pCom->Exists("a", 0));
    EXPECT_FALSE(m_pCom->Exists("b", 0));
    EXPECT_FALSE(m_pCom->Exists("c", 0));
    EXPECT_EQ(m_pCom->m_pMemory, m_pCom->m_Memory);
    EXPECT_EQ(m_pCom->m_OffsetAlloc.GetNrFreeLinks(), 1u);
}

// ---------------------------------------------------------------------------
// Alloc + Free
// ---------------------------------------------------------------------------

TEST_F(CComTest, Alloc_ReturnsValidPointer)
{
    void* pData = m_pCom->Alloc(64);
    EXPECT_NE(pData, nullptr);

    uintptr_t offset = reinterpret_cast<uintptr_t>(pData) - reinterpret_cast<uintptr_t>(m_pCom->m_pMemory);
    EXPECT_GE(offset, 4u);
    EXPECT_LT(offset, 0x4000u);
}

TEST_F(CComTest, Alloc_AlignedTo4Bytes)
{
    void* p1 = m_pCom->Alloc(1);
    void* p2 = m_pCom->Alloc(1);
    void* p3 = m_pCom->Alloc(3);

    uintptr_t addr1 = reinterpret_cast<uintptr_t>(p1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(p2);
    uintptr_t addr3 = reinterpret_cast<uintptr_t>(p3);

    EXPECT_EQ(addr1 % 4, 0u);
    EXPECT_EQ(addr2 % 4, 0u);
    EXPECT_EQ(addr3 % 4, 0u);
}

TEST_F(CComTest, Free_ReleasesMemoryForReuse)
{
    void* p1 = m_pCom->Alloc(100);
    ASSERT_NE(p1, nullptr);

    m_pCom->Free(p1);

    void* p2 = m_pCom->Alloc(100);
    EXPECT_EQ(p1, p2);
}

TEST_F(CComTest, Free_HandlesNullPointer)
{
    EXPECT_NO_FATAL_FAILURE(m_pCom->Free(nullptr));
}

// ---------------------------------------------------------------------------
// CopyAll
// ---------------------------------------------------------------------------

TEST_F(CComTest, CopyAll_CopiesEntriesFromSource)
{
    CCom source;

    CCOMTypeCast* pTypeInt = source.GetTypeCast(CCOM_TYPE_INT32);
    int valInt = 777;
    source.AddValOfType("sourceInt", 10, pTypeInt, sizeof(valInt), reinterpret_cast<const char*>(&valInt));

    CCOMTypeCast* pTypeFloat = source.GetTypeCast(CCOM_TYPE_FLOAT);
    float valFloat = 3.14f;
    source.AddValOfType("sourceFloat", 12, pTypeFloat, sizeof(valFloat), reinterpret_cast<const char*>(&valFloat));

    m_pCom->CopyAll(&source);

    ComValueInfo* pInfoInt = m_pCom->GetpVal("sourceInt", 0);
    ASSERT_NE(pInfoInt, nullptr);
    const char* pDataInt = reinterpret_cast<const char*>(pInfoInt + 1) + pInfoInt->lNameLen;
    EXPECT_EQ(*reinterpret_cast<const int*>(pDataInt), 777);

    ComValueInfo* pInfoFloat = m_pCom->GetpVal("sourceFloat", 0);
    ASSERT_NE(pInfoFloat, nullptr);
    const char* pDataFloat = reinterpret_cast<const char*>(pInfoFloat + 1) + pInfoFloat->lNameLen;
    EXPECT_FLOAT_EQ(*reinterpret_cast<const float*>(pDataFloat), 3.14f);
}

TEST_F(CComTest, CopyAll_HandlesNullSource)
{
    m_pCom->CopyAll(nullptr);
    EXPECT_EQ(m_pCom->m_ComDat.Count(), 0);
}

TEST_F(CComTest, CopyAll_EmptySourceDoesNothing)
{
    CCom source;
    CCom dst;
    dst.CopyAll(&source);
    EXPECT_EQ(dst.m_ComDat.Count(), 0);
}

// ---------------------------------------------------------------------------
// SetVal / GetVal (via CSharedCom interface with CCOMTypeTable)
// ---------------------------------------------------------------------------

TEST_F(CComTest, SetVal_Int)
{
    m_pCom->SetVal("iHealth", 100);
    EXPECT_TRUE(m_pCom->Exists("iHealth", 0));

    int val = 0;
    ASSERT_NE(m_pCom->GetVal("iHealth", &val), 0);
    EXPECT_EQ(val, 100);
}

TEST_F(CComTest, SetVal_Float)
{
    m_pCom->SetVal("fSpeed", 2.5f);
    EXPECT_TRUE(m_pCom->Exists("fSpeed", 0));

    float val = 0.0f;
    ASSERT_NE(m_pCom->GetVal("fSpeed", &val), 0);
    EXPECT_FLOAT_EQ(val, 2.5f);
}

TEST_F(CComTest, SetVal_Bool)
{
    m_pCom->SetVal("bAlive", true);
    EXPECT_TRUE(m_pCom->Exists("bAlive", 0));

    bool val = false;
    ASSERT_NE(m_pCom->GetVal("bAlive", &val), 0);
    EXPECT_TRUE(val);
}

TEST_F(CComTest, SetVal_String)
{
    m_pCom->SetVal("sName", "Agent47");
    EXPECT_TRUE(m_pCom->Exists("sName", 0));

    const char* val = m_pCom->GetVal("sName");
    ASSERT_NE(val, nullptr);
    EXPECT_STREQ(val, "Agent47");
}

TEST_F(CComTest, SetVal_IntWithExplicitType)
{
    m_pCom->SetVal("iPoints", 999, CCOM_TYPE_INT32);
    int val = 0;
    ASSERT_NE(m_pCom->GetVal("iPoints", &val), 0);
    EXPECT_EQ(val, 999);
}

TEST_F(CComTest, SetVal_FloatWithExplicitType)
{
    m_pCom->SetVal("fRatio", 0.75f, CCOM_TYPE_FLOAT);
    float val = 0.0f;
    ASSERT_NE(m_pCom->GetVal("fRatio", &val), 0);
    EXPECT_FLOAT_EQ(val, 0.75f);
}

// ---------------------------------------------------------------------------
// operator[] / Get (via CComRead)
// ---------------------------------------------------------------------------

TEST_F(CComTest, OperatorBracket_ReadsInt)
{
    m_pCom->SetVal("iAmmo", 42);
    int ammo = (*m_pCom)["iAmmo"];
    EXPECT_EQ(ammo, 42);
}

TEST_F(CComTest, OperatorBracket_ReadsFloat)
{
    m_pCom->SetVal("fTime", 12.5f);
    float t = (*m_pCom)["fTime"];
    EXPECT_FLOAT_EQ(t, 12.5f);
}

TEST_F(CComTest, OperatorBracket_ReadsBool)
{
    m_pCom->SetVal("bReady", true);
    bool ready = (*m_pCom)["bReady"];
    EXPECT_TRUE(ready);
}

TEST_F(CComTest, Get_ReadsString)
{
    m_pCom->SetVal("sMsg", "hello world");
    const char* msg = m_pCom->Get("sMsg");
    EXPECT_NE(msg, nullptr);
    EXPECT_STREQ(msg, "hello world");
}

TEST_F(CComTest, OperatorBracket_MissingKeyReturnsZero)
{
    int val = (*m_pCom)["nonExistent"];
    EXPECT_EQ(val, 0);
}

// ---------------------------------------------------------------------------
// PrintStatus (smoke test)
// ---------------------------------------------------------------------------

TEST_F(CComTest, PrintStatus_EmptyCom)
{
    EXPECT_NO_FATAL_FAILURE(m_pCom->PrintStatus());
}

TEST_F(CComTest, PrintStatus_WithEntries)
{
    m_pCom->SetVal("iScore", 500);
    m_pCom->SetVal("fDelta", 0.016f);
    m_pCom->SetVal("bDebug", false);
    EXPECT_NO_FATAL_FAILURE(m_pCom->PrintStatus());
}

// ---------------------------------------------------------------------------
// Stress / Multiple entries
// ---------------------------------------------------------------------------

TEST_F(CComTest, ManyEntries)
{
    const int kCount = 20;
    for (int i = 0; i < kCount; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "iVal_%02d", i);
        m_pCom->SetVal(buf, i * 10);
    }

    for (int i = 0; i < kCount; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "iVal_%02d", i);
        int val = (*m_pCom)[buf];
        EXPECT_EQ(val, i * 10);
    }

    EXPECT_EQ(m_pCom->m_ComDat.Count(), kCount);
}

// ---------------------------------------------------------------------------
// GetDataLen
// ---------------------------------------------------------------------------

TEST_F(CComTest, GetDataLen_ReturnsCorrectSize)
{
    m_pCom->SetVal("iData", 12345);
    EXPECT_EQ(m_pCom->GetDataLen("iData"), static_cast<int>(sizeof(int)));

    m_pCom->SetVal("fData", 1.0f);
    EXPECT_EQ(m_pCom->GetDataLen("fData"), static_cast<int>(sizeof(float)));

    m_pCom->SetVal("sData", "test");
    EXPECT_EQ(m_pCom->GetDataLen("sData"), static_cast<int>(strlen("test") + 1));

    EXPECT_EQ(m_pCom->GetDataLen("missing"), 0);
}

// ---------------------------------------------------------------------------
// GetVal raw buffer
// ---------------------------------------------------------------------------

TEST_F(CComTest, GetVal_RawBuffer)
{
    const char* name = "raw";
    int nameLen = static_cast<int>(strlen(name)) + 1;
    const char* data = "raw_bytes";
    int dataLen = static_cast<int>(strlen(data));

    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_BLOCK);
    m_pCom->AddValOfType(name, nameLen, pType, dataLen, data);

    char out[64] = {};
    int read = m_pCom->GetVal(out, name, 0);
    EXPECT_EQ(read, dataLen);
    EXPECT_EQ(memcmp(out, data, dataLen), 0);
}

// ---------------------------------------------------------------------------
// SetVal raw block
// ---------------------------------------------------------------------------

TEST_F(CComTest, SetVal_RawBlock)
{
    const char* name = "block";
    const char* data = "\x01\x02\x03\x04";
    m_pCom->SetVal(name, static_cast<int>(strlen(name)) + 1, data, 4);

    char out[16] = {};
    int read = m_pCom->GetVal(out, name, 0);
    EXPECT_EQ(read, 4);
    EXPECT_EQ(memcmp(out, data, 4), 0);
}
