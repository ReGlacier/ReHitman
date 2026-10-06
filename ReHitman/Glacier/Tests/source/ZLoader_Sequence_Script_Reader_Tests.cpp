#include <Glacier/LoaderSequence/ZLoader_Sequence_Script_Reader.h>
#include <Glacier/ZUniMemory.h>
#include <gtest/gtest.h>

#include <cstring>

using namespace Glacier;

// These tests exercise ONLY the reading/interpretation logic of
// ZLoader_Sequence_Script_Reader (key-frame counting, picture-name collection,
// attribute parsing and the "fill unset / interpolate" post-pass). No render
// runtime is involved.
//
// The Loader_Sequence scripts quote attribute values (and must, because values
// such as Position carry a comma, which the SimpleXML symbol scanner otherwise
// stops at, and the closing '>' would otherwise be swallowed as the value
// terminator). The scripts below match that real format.
namespace
{
    // Runs the reader over an XML script and owns the output tables the reader
    // allocated into the info struct, freeing them (and the reader scratch) on
    // scope exit.
    struct ScriptParse
    {
        ZLoader_Sequence_Script_Reader reader;
        ZLoader_Sequence_Info info{};

        explicit ScriptParse(const char* pXml)
        {
            reader.Load_Script(&info, pXml, static_cast<uint32_t>(std::strlen(pXml)) + 1);
        }

        ~ScriptParse()
        {
            reader.Clear();
            ZUniMemory::Free(info.m_pKey_Frame_Info_Table);
            ZUniMemory::Free(info.m_pPicture_Info_Table);
            ZUniMemory::Free(info.m_pPicture_Name_Buffer);
            ZUniMemory::Free(info.m_pPicture_Name_Offsets_Buffer);
        }

        const char* PictureName(uint32_t iPicture_Nr) const
        {
            return &info.m_pPicture_Name_Buffer[info.m_pPicture_Name_Offsets_Buffer[iPicture_Nr]];
        }

        // Picture table is row-major, strided by the picture count.
        const SPicture_Info& Pic(uint32_t iPicture_Nr, uint32_t iKey_Frame) const
        {
            return info.m_pPicture_Info_Table[iPicture_Nr + info.m_iPicture_Name_Count * iKey_Frame];
        }

        float KeyTime(uint32_t iKey_Frame) const
        {
            return info.m_pKey_Frame_Info_Table[iKey_Frame].m_fTime;
        }
    };
}

TEST(ZLoader_Sequence_Script_Reader, CountsKeysPicturesAndInterpolatesUnsetColumns)
{
    ScriptParse p(
        "<Loader_Sequence_Script>"
        "<Key_Frame Time=\"0\"><Picture_Settings Picture=\"bg\" Position=\"10,20\" Opacity=\"0.5\" Multiply=\"2\" Position_Interpolation=\"Time\"/></Key_Frame>"
        "<Key_Frame Time=\"1\"></Key_Frame>"
        "<Key_Frame Time=\"2\"><Picture_Settings Picture=\"bg\" Position=\"100,20\"/></Key_Frame>"
        "</Loader_Sequence_Script>");

    EXPECT_EQ(p.info.m_iNr_Key_Frames, 3u);
    EXPECT_EQ(p.info.m_iPicture_Name_Count, 1u);
    EXPECT_STREQ(p.PictureName(0), "bg");

    EXPECT_FLOAT_EQ(p.KeyTime(0), 0.0f);
    EXPECT_FLOAT_EQ(p.KeyTime(1), 1.0f);
    EXPECT_FLOAT_EQ(p.KeyTime(2), 2.0f);

    // Explicit first key frame.
    EXPECT_EQ(p.Pic(0, 0).m_iPicture_Nr, 0u);
    EXPECT_FLOAT_EQ(p.Pic(0, 0).m_fPosX, 10.0f);
    EXPECT_FLOAT_EQ(p.Pic(0, 0).m_fPosY, 20.0f);
    EXPECT_FLOAT_EQ(p.Pic(0, 0).m_fOpacity, 0.5f);
    EXPECT_FLOAT_EQ(p.Pic(0, 0).m_fMultiply, 2.0f);
    EXPECT_EQ(p.Pic(0, 0).m_iPosition_Interpolation, 0u); // 0 = Time

    // Middle key frame had no Picture_Settings: position interpolated between
    // (10 @ t0) and (100 @ t2) => 55 @ t1; opacity/multiply carried forward.
    EXPECT_FLOAT_EQ(p.Pic(0, 1).m_fPosX, 55.0f);
    EXPECT_FLOAT_EQ(p.Pic(0, 1).m_fPosY, 20.0f);
    EXPECT_FLOAT_EQ(p.Pic(0, 1).m_fOpacity, 0.5f);
    EXPECT_FLOAT_EQ(p.Pic(0, 1).m_fMultiply, 2.0f);

    // Last key frame explicit position; opacity/multiply still carried.
    EXPECT_FLOAT_EQ(p.Pic(0, 2).m_fPosX, 100.0f);
    EXPECT_FLOAT_EQ(p.Pic(0, 2).m_fPosY, 20.0f);
    EXPECT_FLOAT_EQ(p.Pic(0, 2).m_fOpacity, 0.5f);
    EXPECT_FLOAT_EQ(p.Pic(0, 2).m_fMultiply, 2.0f);
}

TEST(ZLoader_Sequence_Script_Reader, InsertsImplicitKeyFrameAtZero)
{
    // A single non-zero key frame forces an implicit key frame at time 0, so the
    // parsed count is 2 even though the script only has one Key_Frame element.
    ScriptParse p(
        "<Loader_Sequence_Script>"
        "<Key_Frame Time=\"0.5\"><Picture_Settings Picture=\"logo\" Position=\"7,8\"/></Key_Frame>"
        "</Loader_Sequence_Script>");

    EXPECT_EQ(p.info.m_iNr_Key_Frames, 2u);
    EXPECT_FLOAT_EQ(p.KeyTime(0), 0.0f);
    EXPECT_FLOAT_EQ(p.KeyTime(1), 0.5f);

    // Row for the implicit key frame is default-filled (position 0, opacity 1).
    EXPECT_FLOAT_EQ(p.Pic(0, 0).m_fPosX, 0.0f);
    EXPECT_FLOAT_EQ(p.Pic(0, 0).m_fPosY, 0.0f);
    EXPECT_FLOAT_EQ(p.Pic(0, 0).m_fOpacity, 1.0f);

    // Row for the real key frame carries the parsed settings.
    EXPECT_FLOAT_EQ(p.Pic(0, 1).m_fPosX, 7.0f);
    EXPECT_FLOAT_EQ(p.Pic(0, 1).m_fPosY, 8.0f);

    // No Screen element -> engine defaults; Full_Progress_Time falls back to the
    // last key frame time.
    EXPECT_FLOAT_EQ(p.reader.m_fScreen_Size_X, 640.0f);
    EXPECT_FLOAT_EQ(p.reader.m_fScreen_Size_Y, 400.0f);
    EXPECT_FLOAT_EQ(p.reader.m_fFull_Progress_Time, 0.5f);
}

TEST(ZLoader_Sequence_Script_Reader, ReadsScreenSizeAndFullProgressTime)
{
    ScriptParse p(
        "<Loader_Sequence_Script>"
        "<Screen Screen_Size=\"800,600\" Full_Progress_Time=\"5\"/>"
        "<Key_Frame Time=\"1\"><Picture_Settings Picture=\"a\"/></Key_Frame>"
        "</Loader_Sequence_Script>");

    EXPECT_FLOAT_EQ(p.reader.m_fScreen_Size_X, 800.0f);
    EXPECT_FLOAT_EQ(p.reader.m_fScreen_Size_Y, 600.0f);
    EXPECT_FLOAT_EQ(p.reader.m_fFull_Progress_Time, 5.0f);
    EXPECT_STREQ(p.PictureName(0), "a");
}

TEST(ZLoader_Sequence_Script_Reader, PositionWithoutCommaDuplicatesAxis)
{
    // Get_Cds with a missing comma keeps both halves equal to the full value.
    ScriptParse p(
        "<Loader_Sequence_Script>"
        "<Key_Frame Time=\"0\"><Picture_Settings Picture=\"p\" Position=\"42\"/></Key_Frame>"
        "</Loader_Sequence_Script>");

    EXPECT_FLOAT_EQ(p.Pic(0, 0).m_fPosX, 42.0f);
    EXPECT_FLOAT_EQ(p.Pic(0, 0).m_fPosY, 42.0f);
}

TEST(ZLoader_Sequence_Script_Reader, PictureNamesDeduplicateCaseInsensitively)
{
    ScriptParse p(
        "<Loader_Sequence_Script>"
        "<Key_Frame Time=\"0\"><Picture_Settings Picture=\"bg\"/></Key_Frame>"
        "<Key_Frame Time=\"1\"><Picture_Settings Picture=\"BG\"/></Key_Frame>"
        "</Loader_Sequence_Script>");

    EXPECT_EQ(p.info.m_iPicture_Name_Count, 1u);
    EXPECT_STREQ(p.PictureName(0), "bg");
}

TEST(ZLoader_Sequence_Script_Reader, ProgressInterpolationModeIsParsed)
{
    ScriptParse p(
        "<Loader_Sequence_Script>"
        "<Key_Frame Time=\"0\"><Picture_Settings Picture=\"p\" Position_Interpolation=\"Progress\"/></Key_Frame>"
        "<Key_Frame Time=\"1\"><Picture_Settings Picture=\"p\" Position_Interpolation=\"Time\"/></Key_Frame>"
        "</Loader_Sequence_Script>");

    EXPECT_EQ(p.Pic(0, 0).m_iPosition_Interpolation, 1u); // 1 = Progress
    EXPECT_EQ(p.Pic(0, 1).m_iPosition_Interpolation, 0u); // 0 = Time
}
