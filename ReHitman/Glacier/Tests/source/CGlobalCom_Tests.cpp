#include <gtest/gtest.h>
#include <Glacier/Com/CGlobalCom.h>
#include <Glacier/Com/CCOMType.h>
#include <Glacier/Com/CCOMTypeCast.h>
#include <Glacier/Com/CCOMFormat.h>
#include <cstring>
#include <cstdio>

using namespace Glacier;

namespace Glacier { extern CCOMTypeCast CCOMTypeTable[18]; }

static void InitCCOMTypeTable()
{
    auto& t = CCOMTypeTable;

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

class CGlobalComTest : public ::testing::Test
{
protected:
    static constexpr int kBufferSize = 4096;

    CGlobalComTest()
    {
        InitCCOMTypeTable();
    }

    void SetUp() override
    {
        memset(m_Buffer, 0, sizeof(m_Buffer));
        m_pCom = new CGlobalCom(m_Buffer, kBufferSize);
    }

    void TearDown() override
    {
        delete m_pCom;
        m_pCom = nullptr;
    }

    uint8_t m_Buffer[kBufferSize];
    CGlobalCom* m_pCom = nullptr;
};

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, Constructor_WithBuffer_InitializesMembers)
{
    EXPECT_EQ(m_pCom->maxSize, kBufferSize);
    EXPECT_EQ(m_pCom->memory, m_Buffer);
    EXPECT_EQ(m_pCom->memoryWritePtr, 0);
}

TEST_F(CGlobalComTest, Constructor_Default_InitializesWithNull)
{
    CGlobalCom com;
    EXPECT_EQ(com.maxSize, 0);
    EXPECT_EQ(com.memory, nullptr);
    EXPECT_EQ(com.memoryWritePtr, 0);
}

// ---------------------------------------------------------------------------
// AddValOfType + GetpVal
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, AddValOfType_StoresAndRetrievesValue)
{
    const char* name = "level";
    int nameLen = static_cast<int>(strlen(name)) + 1;
    int value = 10;

    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_INT32);
    ASSERT_NE(pType, nullptr);

    m_pCom->AddValOfType(name, nameLen, pType, sizeof(value), reinterpret_cast<const char*>(&value));

    ComValueInfo* pInfo = m_pCom->GetpVal(name, 0);
    ASSERT_NE(pInfo, nullptr);
    EXPECT_EQ(pInfo->lNameLen, nameLen);
    EXPECT_EQ(pInfo->lDataLen, static_cast<int>(sizeof(value)));

    const char* pStoredName = reinterpret_cast<const char*>(pInfo + 1);
    EXPECT_STREQ(pStoredName, name);

    const int* pStoredData = reinterpret_cast<const int*>(pStoredName + nameLen);
    EXPECT_EQ(*pStoredData, value);
}

TEST_F(CGlobalComTest, GetpVal_ReturnsNullForMissingKey)
{
    EXPECT_EQ(m_pCom->GetpVal("missing", 0), nullptr);
}

// ---------------------------------------------------------------------------
// Exists
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, Exists_ReturnsTrueForExistingKey)
{
    const char* name = "flag";
    int nameLen = static_cast<int>(strlen(name)) + 1;
    bool val = true;

    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_BOOL);
    m_pCom->AddValOfType(name, nameLen, pType, sizeof(val), reinterpret_cast<const char*>(&val));

    EXPECT_TRUE(m_pCom->Exists(name, 0));
}

TEST_F(CGlobalComTest, Exists_ReturnsFalseForMissingKey)
{
    EXPECT_FALSE(m_pCom->Exists("phantom", 0));
}

// ---------------------------------------------------------------------------
// RemoveVal
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, RemoveVal_MarksAsUnused)
{
    const char* name = "gravity";
    int nameLen = static_cast<int>(strlen(name)) + 1;
    float val = 9.8f;

    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_FLOAT);
    m_pCom->AddValOfType(name, nameLen, pType, sizeof(val), reinterpret_cast<const char*>(&val));

    EXPECT_TRUE(m_pCom->Exists(name, 0));

    bool removed = m_pCom->RemoveVal(name, 0);
    EXPECT_TRUE(removed);
    EXPECT_FALSE(m_pCom->Exists(name, 0));
}

TEST_F(CGlobalComTest, RemoveVal_ReturnsFalseForMissingKey)
{
    EXPECT_FALSE(m_pCom->RemoveVal("nope", 0));
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, Clear_ResetsWritePointer)
{
    m_pCom->SetVal("iA", 1);
    m_pCom->SetVal("iB", 2);
    EXPECT_GT(m_pCom->memoryWritePtr, 0);

    m_pCom->Clear();
    EXPECT_EQ(m_pCom->memoryWritePtr, 0);
    EXPECT_FALSE(m_pCom->Exists("iA", 0));
    EXPECT_FALSE(m_pCom->Exists("iB", 0));
}

// ---------------------------------------------------------------------------
// Append (in-place overwrite / new entry)
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, Append_OverwritesInPlaceWhenSameSize)
{
    const char* name = "count";
    int nameLen = static_cast<int>(strlen(name)) + 1;
    int firstVal = 100;
    int secondVal = 200;

    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_INT32);

    m_pCom->AddValOfType(name, nameLen, pType, sizeof(firstVal), reinterpret_cast<const char*>(&firstVal));
    int oldWritePtr = m_pCom->memoryWritePtr;

    m_pCom->AddValOfType(name, nameLen, pType, sizeof(secondVal), reinterpret_cast<const char*>(&secondVal));

    EXPECT_EQ(m_pCom->memoryWritePtr, oldWritePtr);

    ComValueInfo* pInfo = m_pCom->GetpVal(name, 0);
    ASSERT_NE(pInfo, nullptr);
    const char* pData = reinterpret_cast<const char*>(pInfo + 1) + pInfo->lNameLen;
    EXPECT_EQ(*reinterpret_cast<const int*>(pData), secondVal);
}

// ---------------------------------------------------------------------------
// Find
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, Find_ReturnsHeaderForExistingKey)
{
    m_pCom->SetVal("sName", "Agent");
    auto* pFound = m_pCom->Find("sName", 0);
    ASSERT_NE(pFound, nullptr);
    EXPECT_TRUE(pFound->used);
}

TEST_F(CGlobalComTest, Find_ReturnsNullForMissingKey)
{
    EXPECT_EQ(m_pCom->Find("ghost", 0), nullptr);
}

TEST_F(CGlobalComTest, Find_ReturnsNullWhenEmpty)
{
    CGlobalCom com(m_Buffer, kBufferSize);
    EXPECT_EQ(com.Find("anything", 0), nullptr);
}

// ---------------------------------------------------------------------------
// Defrag
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, Defrag_ReclaimsSpaceFromUnusedEntries)
{
    m_pCom->SetVal("iFirst", 1);
    m_pCom->SetVal("iSecond", 2);
    m_pCom->SetVal("iThird", 3);

    int writePtrBeforeRemoval = m_pCom->memoryWritePtr;

    m_pCom->RemoveVal("iSecond", 0);

    EXPECT_EQ(m_pCom->memoryWritePtr, writePtrBeforeRemoval);

    m_pCom->Defrag();

    EXPECT_LT(m_pCom->memoryWritePtr, writePtrBeforeRemoval);

    EXPECT_TRUE(m_pCom->Exists("iFirst", 0));
    EXPECT_TRUE(m_pCom->Exists("iThird", 0));
    EXPECT_FALSE(m_pCom->Exists("iSecond", 0));
}

TEST_F(CGlobalComTest, Defrag_LeavesOnlyLiveEntries)
{
    m_pCom->SetVal("iA", 10);
    m_pCom->SetVal("iB", 20);
    m_pCom->SetVal("iC", 30);
    m_pCom->RemoveVal("iA", 0);
    m_pCom->RemoveVal("iC", 0);

    m_pCom->Defrag();

    EXPECT_TRUE(m_pCom->Exists("iB", 0));
    EXPECT_FALSE(m_pCom->Exists("iA", 0));
    EXPECT_FALSE(m_pCom->Exists("iC", 0));

    int val = (*m_pCom)["iB"];
    EXPECT_EQ(val, 20);
}

TEST_F(CGlobalComTest, Defrag_NoUnusedEntries_NoChange)
{
    m_pCom->SetVal("iOne", 1);
    m_pCom->SetVal("iTwo", 2);

    int writePtr = m_pCom->memoryWritePtr;
    m_pCom->Defrag();
    EXPECT_EQ(m_pCom->memoryWritePtr, writePtr);
}

// ---------------------------------------------------------------------------
// EnumKeys
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, EnumKeys_EnumeratesAllKeys)
{
    m_pCom->SetVal("iAlpha", 1);
    m_pCom->SetVal("iBeta", 2);
    m_pCom->SetVal("iGamma", 3);

    char buf[128];

    EXPECT_TRUE(m_pCom->EnumKeys(buf, sizeof(buf), 0));
    EXPECT_STREQ(buf, "iAlpha");

    EXPECT_TRUE(m_pCom->EnumKeys(buf, sizeof(buf), 1));
    EXPECT_STREQ(buf, "iBeta");

    EXPECT_TRUE(m_pCom->EnumKeys(buf, sizeof(buf), 2));
    EXPECT_STREQ(buf, "iGamma");

    EXPECT_FALSE(m_pCom->EnumKeys(buf, sizeof(buf), 3));
}

TEST_F(CGlobalComTest, EnumKeys_SkipsRemovedEntries)
{
    m_pCom->SetVal("iFirst", 1);
    m_pCom->SetVal("iSecond", 2);
    m_pCom->SetVal("iThird", 3);

    m_pCom->RemoveVal("iSecond", 0);

    char buf[128];

    EXPECT_TRUE(m_pCom->EnumKeys(buf, sizeof(buf), 0));
    EXPECT_STREQ(buf, "iFirst");

    EXPECT_TRUE(m_pCom->EnumKeys(buf, sizeof(buf), 1));
    EXPECT_STREQ(buf, "iThird");

    EXPECT_FALSE(m_pCom->EnumKeys(buf, sizeof(buf), 2));
}

TEST_F(CGlobalComTest, EnumKeys_EmptyComReturnsFalse)
{
    char buf[64];
    EXPECT_FALSE(m_pCom->EnumKeys(buf, sizeof(buf), 0));
}

// ---------------------------------------------------------------------------
// SetVal / GetVal (via CSharedCom interface)
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, SetVal_Int)
{
    m_pCom->SetVal("iScore", 5000);
    int val = 0;
    ASSERT_NE(m_pCom->GetVal("iScore", &val), 0);
    EXPECT_EQ(val, 5000);
}

TEST_F(CGlobalComTest, SetVal_Float)
{
    m_pCom->SetVal("fTime", 1.25f);
    float val = 0.0f;
    ASSERT_NE(m_pCom->GetVal("fTime", &val), 0);
    EXPECT_FLOAT_EQ(val, 1.25f);
}

TEST_F(CGlobalComTest, SetVal_Bool)
{
    m_pCom->SetVal("bActive", true);
    bool val = false;
    ASSERT_NE(m_pCom->GetVal("bActive", &val), 0);
    EXPECT_TRUE(val);
}

TEST_F(CGlobalComTest, SetVal_String)
{
    m_pCom->SetVal("sMission", "Requiem");
    const char* val = m_pCom->GetVal("sMission");
    ASSERT_NE(val, nullptr);
    EXPECT_STREQ(val, "Requiem");
}

TEST_F(CGlobalComTest, SetVal_Overwrite)
{
    m_pCom->SetVal("iX", 10);
    m_pCom->SetVal("iX", 20);

    int val = 0;
    m_pCom->GetVal("iX", &val);
    EXPECT_EQ(val, 20);
}

// ---------------------------------------------------------------------------
// operator[] / Get
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, OperatorBracket_ReadsValue)
{
    m_pCom->SetVal("iGold", 9999);
    int gold = (*m_pCom)["iGold"];
    EXPECT_EQ(gold, 9999);
}

TEST_F(CGlobalComTest, OperatorBracket_MissingKeyReturnsZero)
{
    int val = (*m_pCom)["noKey"];
    EXPECT_EQ(val, 0);
}

// ---------------------------------------------------------------------------
// GetDataLen
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, GetDataLen_ReturnsSize)
{
    m_pCom->SetVal("iVal", 42);
    EXPECT_EQ(m_pCom->GetDataLen("iVal"), static_cast<int>(sizeof(int)));

    m_pCom->SetVal("sStr", "hello");
    EXPECT_EQ(m_pCom->GetDataLen("sStr"), static_cast<int>(strlen("hello") + 1));
}

// ---------------------------------------------------------------------------
// Stress / Capacity
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, ManyEntries)
{
    const int kCount = 50;
    for (int i = 0; i < kCount; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "iKey_%02d", i);
        m_pCom->SetVal(buf, i * 100);
    }

    for (int i = 0; i < kCount; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "iKey_%02d", i);
        EXPECT_TRUE(m_pCom->Exists(buf, 0));

        int val = (*m_pCom)[buf];
        EXPECT_EQ(val, i * 100);
    }
}

TEST_F(CGlobalComTest, ManyEntries_WithDefrag)
{
    for (int i = 0; i < 30; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "iItem_%02d", i);
        m_pCom->SetVal(buf, i);
    }

    for (int i = 0; i < 30; i += 2)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "iItem_%02d", i);
        m_pCom->RemoveVal(buf, 0);
    }

    m_pCom->Defrag();

    for (int i = 1; i < 30; i += 2)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "iItem_%02d", i);
        EXPECT_TRUE(m_pCom->Exists(buf, 0));
        int val = (*m_pCom)[buf];
        EXPECT_EQ(val, i);
    }
}

// ---------------------------------------------------------------------------
// Raw block GetVal
// ---------------------------------------------------------------------------

TEST_F(CGlobalComTest, GetVal_RawBuffer)
{
    const char* name = "raw";
    int nameLen = static_cast<int>(strlen(name)) + 1;
    const char* data = "raw_data";
    int dataLen = static_cast<int>(strlen(data));

    CCOMTypeCast* pType = m_pCom->GetTypeCast(CCOM_TYPE_BLOCK);
    m_pCom->AddValOfType(name, nameLen, pType, dataLen, data);

    char out[64] = {};
    int read = m_pCom->GetVal(out, name, 0);
    EXPECT_EQ(read, dataLen);
    EXPECT_EQ(memcmp(out, data, dataLen), 0);
}
