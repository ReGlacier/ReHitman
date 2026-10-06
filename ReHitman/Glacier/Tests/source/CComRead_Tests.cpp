#include <gtest/gtest.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Com/CComRead.h>
#include <Glacier/Com/CCOMType.h>
#include <Glacier/Com/CCOMTypeCast.h>
#include <Glacier/Com/CCOMFormat.h>
#include <cstring>

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

class CComReadTest : public ::testing::Test
{
protected:
    CComReadTest()
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
// Constructor
// ---------------------------------------------------------------------------

TEST_F(CComReadTest, Constructor_BindsToNameAndCom)
{
    CComRead reader(m_pCom, "testKey");
    EXPECT_EQ(reader.m_pCom, m_pCom);
    EXPECT_STREQ(reader.m_pName, "testKey");
}

TEST_F(CComReadTest, CopyConstructor_ShallowCopy)
{
    CComRead reader(m_pCom, "original");
    CComRead copy(reader);

    EXPECT_EQ(copy.m_pCom, reader.m_pCom);
    EXPECT_EQ(copy.m_pName, reader.m_pName);
}

// ---------------------------------------------------------------------------
// operator int32_t
// ---------------------------------------------------------------------------

TEST_F(CComReadTest, OperatorInt32_ReadsValue)
{
    m_pCom->SetVal("iScore", 999);

    CComRead reader = m_pCom->Get("iScore");
    int val = reader;
    EXPECT_EQ(val, 999);
}

TEST_F(CComReadTest, OperatorInt32_MissingKeyReturnsZero)
{
    CComRead reader(m_pCom, "noSuchInt");
    int val = reader;
    EXPECT_EQ(val, 0);
}

// ---------------------------------------------------------------------------
// operator uint32_t
// ---------------------------------------------------------------------------

TEST_F(CComReadTest, OperatorUInt32_ReadsValue)
{
    m_pCom->SetVal("iFlags", 0x7FFFFFFF);

    CComRead reader = (*m_pCom)["iFlags"];
    uint32_t val = reader;
    EXPECT_EQ(val, 0x7FFFFFFFu);
}

TEST_F(CComReadTest, OperatorUInt32_NegativeValueWraps)
{
    m_pCom->SetVal("iNeg", -1);

    CComRead reader = m_pCom->Get("iNeg");
    uint32_t val = reader;
    EXPECT_EQ(val, 0xFFFFFFFFu);
}

// ---------------------------------------------------------------------------
// operator float
// ---------------------------------------------------------------------------

TEST_F(CComReadTest, OperatorFloat_ReadsValue)
{
    m_pCom->SetVal("fSpeed", 3.14f);

    CComRead reader = (*m_pCom)["fSpeed"];
    float val = reader;
    EXPECT_FLOAT_EQ(val, 3.14f);
}

TEST_F(CComReadTest, OperatorFloat_MissingKeyReturnsZero)
{
    CComRead reader(m_pCom, "noFloat");
    float val = reader;
    EXPECT_FLOAT_EQ(val, 0.0f);
}

// ---------------------------------------------------------------------------
// operator char*
// ---------------------------------------------------------------------------

TEST_F(CComReadTest, OperatorCharPtr_ReadsString)
{
    m_pCom->SetVal("sName", "Agent 47");

    CComRead reader = m_pCom->Get("sName");
    char* val = reader;
    ASSERT_NE(val, nullptr);
    EXPECT_STREQ(val, "Agent 47");
}

TEST_F(CComReadTest, OperatorCharPtr_MissingKeyReturnsNull)
{
    CComRead reader(m_pCom, "noString");
    char* val = reader;
    EXPECT_EQ(val, nullptr);
}

// ---------------------------------------------------------------------------
// operator bool
// ---------------------------------------------------------------------------

TEST_F(CComReadTest, OperatorBool_TrueValue)
{
    m_pCom->SetVal("bEnabled", true);

    CComRead reader = (*m_pCom)["bEnabled"];
    bool val = reader;
    EXPECT_TRUE(val);
}

TEST_F(CComReadTest, OperatorBool_FalseValue)
{
    m_pCom->SetVal("bDebug", false);

    CComRead reader = m_pCom->Get("bDebug");
    bool val = reader;
    EXPECT_FALSE(val);
}

TEST_F(CComReadTest, OperatorBool_MissingKeyReturnsFalse)
{
    CComRead reader(m_pCom, "noBool");
    bool val = reader;
    EXPECT_FALSE(val);
}

// ---------------------------------------------------------------------------
// CSharedCom::Get / operator[]
// ---------------------------------------------------------------------------

TEST_F(CComReadTest, CSharedCom_Get_ReturnsCComRead)
{
    m_pCom->SetVal("iValue", 42);
    int val = m_pCom->Get("iValue");
    EXPECT_EQ(val, 42);
}

TEST_F(CComReadTest, CSharedCom_OperatorBracket_ReturnsCComRead)
{
    m_pCom->SetVal("iValue", 77);
    int val = (*m_pCom)["iValue"];
    EXPECT_EQ(val, 77);
}

TEST_F(CComReadTest, CSharedCom_OperatorBracket_ChainsAsBool)
{
    m_pCom->SetVal("bOk", true);
    if ((*m_pCom)["bOk"])
    {
        SUCCEED();
    }
    else
    {
        FAIL() << "Expected true from operator[] -> operator bool()";
    }
}

// ---------------------------------------------------------------------------
// Mixed type reads
// ---------------------------------------------------------------------------

TEST_F(CComReadTest, ReadIntAsFloat_DoesNotCrash)
{
    m_pCom->SetVal("iData", 42);
    CComRead reader = m_pCom->Get("iData");
    float val = reader;
    (void)val;
    SUCCEED();
}

TEST_F(CComReadTest, ReadFloatAsInt_DoesNotCrash)
{
    m_pCom->SetVal("fData", 1.5f);
    CComRead reader = m_pCom->Get("fData");
    int val = reader;
    (void)val;
    SUCCEED();
}
