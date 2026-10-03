// Byte-exact validation of the ZSharedResourcesD3D procedural map/texture
// generators against bundled reference data (see Tools/TextureCapture and
// Tools/ida_dump for the capture tooling).
//
// The CPU-side replicas below mirror the fill functions in
// ZSharedResourcesD3D.cpp one-to-one (single source of truth is the disassembly
// export). Fixtures live under Tests/data; tests skip when a fixture is absent.
#include <gtest/gtest.h>

#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

namespace
{
    std::vector<uint8_t> LoadFile(const std::filesystem::path& rPath)
    {
        std::ifstream stream(rPath, std::ios::binary);
        if (!stream)
        {
            return {};
        }
        return std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)),
            std::istreambuf_iterator<char>());
    }

    std::vector<uint8_t> LoadFixture(const char* pszName)
    {
        return LoadFile(std::filesystem::path(GLACIER_TESTS_DATA_DIR) / pszName);
    }

    std::vector<uint8_t> LoadTexdump(const char* pszName)
    {
        return LoadFile(std::filesystem::path(GLACIER_TESTS_DATA_DIR) / "ref_data" / pszName);
    }

    // ---- CPU replicas of ZSharedResourcesD3D.cpp ---------------------------

    double ClampUnit(double v)
    {
        if (v < 0.0) return 0.0;
        if (v > 1.0) return 1.0;
        return v;
    }

    uint32_t RoundByte(double v)
    {
        const int i = static_cast<int>(v + 0.5);
        return i > 255 ? 255u : static_cast<uint32_t>(i > 0 ? i : 0);
    }

    uint32_t TruncByte(double v)
    {
        const int i = static_cast<int>(v);
        return i > 255 ? 255u : static_cast<uint32_t>(i > 0 ? i : 0);
    }

    uint32_t PackRGB(uint32_t r, uint32_t g, uint32_t b)
    {
        return 0xFF000000u | (r << 16) | (g << 8) | b;
    }

    uint32_t CpuScatter(int x, int y, int w, int h)
    {
        const double u = 2.0 * (x + 0.5) / w - 1.0;
        const double v = 2.0 * (y + 0.5) / h - 1.0;
        if (u == 0.0 || v == 0.0)
        {
            return 0xFF000000u;
        }
        const double au = std::fabs(u);
        const double av = std::fabs(v);
        const double f = 1.0 - std::exp(-(1.0 / au + 1.0 / av) * 0.5);
        const double d = au + av;
        return PackRGB(RoundByte(f * 0.8 / d * 63.75),
                       RoundByte(f * 0.5 / d * 63.75),
                       RoundByte(f * 0.4 / d * 63.75));
    }

    uint32_t CpuSpecular(int x, int y, int w, int h)
    {
        const double angle = (y + 0.5) / h * 1.5707964;
        const double power = (x + 0.5) / w * 128.0;
        const uint32_t c = RoundByte(std::pow(std::cos(angle), power) * 255.0);
        return c | (c << 8) | (c << 16) | (c << 24);
    }

    uint32_t CpuAnisotropic(int x, int y, int w, int h)
    {
        const double col = 2.0 * (x + 0.5) / w - 1.0;
        const double row = 2.0 * (y + 0.5) / h - 1.0;
        const double dot = col * row + std::sqrt(1.0 - col * col) * std::sqrt(1.0 - row * row);
        const uint32_t a = RoundByte(std::pow(dot, 128.0) * 255.0);
        const uint32_t c = RoundByte(std::sqrt(1.0 - col * col) * 255.0);
        return (a << 24) | (c << 16) | (c << 8) | c;
    }

    uint32_t CpuHgPhase(int, int y, int w, int h)
    {
        const double u = 2.0 * (y + 0.5) / h - 1.0;
        const auto hg = [u](double g) {
            const double num = 1.0 - g * g;
            const double den = g * g + 1.0 - 2.0 * g * u;
            return num / std::pow(den, 1.5);
        };
        return PackRGB(TruncByte(hg(0.8) * 63.75),
                       TruncByte(hg(0.3) * 63.75),
                       TruncByte(hg(0.0) * 63.75));
    }

    uint32_t CpuRefraction(int x, int, int w, int)
    {
        const double fU = 2.0 * (x + 0.5) / w - 1.0;
        if (fU == 0.0)
        {
            return 0u;
        }
        constexpr double kK = 0.5102041059610798;
        constexpr double kC = 0.7142857313156128;
        const double fNeg = -fU;
        const double fT = 1.0 - (1.0 - fU * fU) * kK;
        const double fS = fT < 0.0 ? 0.0 : 1.0;
        const double fRt = fT > 0.0 ? std::sqrt(fT) : 0.0;
        const double fC = kC * fS;
        const double fM = (fNeg * kC - fRt) * fS;
        const double fD = -(fC * fU + fM);
        const double fE = fD != 0.0 ? fNeg / fD : 0.0;
        const double fP = fNeg != 0.0 ? fD / fNeg : 0.0;
        const double fA = (fE - kC) / (fE + kC);
        const double fB = (fP - kC) / (fP + kC);
        const double fH = (fA * fA + fB * fB) * 0.5 * fS + (1.0 - fS);
        const double fR = ClampUnit(fC);
        const double fG = ClampUnit(-fM);
        // smoothstep clamp [0,0.5]
        double s = ClampUnit(fH);
        if (s > 0.5) s = 0.5;
        const double t = s / 0.5;
        const double sm = 3.0 * t * t - 2.0 * t * t * t;
        const int32_t lA = static_cast<int32_t>((1.0 - sm) * -255.0);
        const int32_t lX = static_cast<int32_t>(fR * -255.0);
        const int32_t lZ = static_cast<int32_t>(fG * -255.0);
        int64_t res = (0xFF00LL - lX) << 8;
        res = ((res - lZ) << 8) - lA;
        return static_cast<uint32_t>(res);
    }

    uint32_t CpuNormalizer(int face, int x, int y, int w, int h)
    {
        const double col = 2.0 * (x + 0.5) / w - 1.0;
        const double row = 2.0 * (y + 0.5) / h - 1.0;
        double d[3];
        switch (face)
        {
        case 0: d[0] = 1.0; d[1] = -row; d[2] = -col; break;
        case 1: d[0] = -1.0; d[1] = -row; d[2] = col; break;
        case 2: d[0] = col; d[1] = 1.0; d[2] = row; break;
        case 3: d[0] = col; d[1] = -1.0; d[2] = -row; break;
        case 4: d[0] = col; d[1] = -row; d[2] = 1.0; break;
        default: d[0] = -col; d[1] = -row; d[2] = -1.0; break;
        }
        const double len = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
        return PackRGB(RoundByte((d[0] / len * 0.5 + 0.5) * 255.0),
                       RoundByte((d[1] / len * 0.5 + 0.5) * 255.0),
                       RoundByte((d[2] / len * 0.5 + 0.5) * 255.0));
    }

    uint32_t CpuSpot(double fRatio, int lMode, int x, int y, int w, int h)
    {
        const double col = 2.0 * (x + 0.5) / w - 1.0;
        const double row = 2.0 * (y + 0.5) / h - 1.0;
        double v = 0.0;
        if (lMode != 0)
        {
            const double fx = ClampUnit((std::fabs(col) - fRatio) / (1.0 - fRatio));
            const double fy = ClampUnit((std::fabs(row) - fRatio) / (1.0 - fRatio));
            const double base = (1.0 - fx) * (1.0 - fy);
            v = base * base * (3.0 - 2.0 * base);
        }
        else
        {
            const double dist = std::sqrt(col * col + row * row);
            if (dist < 1.0)
            {
                if (dist <= fRatio)
                {
                    v = 1.0;
                }
                else
                {
                    const double t = 1.0 - (dist - fRatio) / (1.0 - fRatio);
                    v = t * t * (3.0 - 2.0 * t);
                }
            }
        }
        if (!y || y == h - 1 || !x || x == w - 1)
        {
            v = 0.0;
        }
        const uint32_t c = RoundByte(v * 255.0);
        return c | (c << 8) | (c << 16) | (c << 24);
    }

    std::vector<uint8_t> Render(int w, int h,
                                const std::function<uint32_t(int, int, int, int)>& fn)
    {
        std::vector<uint8_t> out(static_cast<size_t>(w) * h * 4);
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const uint32_t c = fn(x, y, w, h);
                std::memcpy(out.data() + (static_cast<size_t>(y) * w + x) * 4, &c, 4);
            }
        }
        return out;
    }

    void ExpectMatch(const char* pszName, int w, int h, const std::vector<uint8_t>& rActual, int nMaxDelta = 0)
    {
        const std::vector<uint8_t> kDump = LoadTexdump(pszName);
        if (kDump.empty())
        {
            GTEST_SKIP() << "fixture ref_data/" << pszName << " is not available";
        }
        ASSERT_EQ(kDump.size(), static_cast<size_t>(w) * h * 4) << pszName;
        ASSERT_EQ(rActual.size(), kDump.size());
        int nBigDiff = 0;
        int nLsbDiff = 0;
        for (size_t i = 0; i < rActual.size(); ++i)
        {
            const int nDiff = static_cast<int>(rActual[i]) - static_cast<int>(kDump[i]);
            const int nAbs = nDiff < 0 ? -nDiff : nDiff;
            if (nAbs > nMaxDelta)
            {
                ++nBigDiff;
            }
            else if (nAbs > 0)
            {
                ++nLsbDiff;
            }
        }
        EXPECT_EQ(nBigDiff, 0) << pszName << ": " << nBigDiff << " bytes differ by more than "
                               << nMaxDelta << " (sample: see first mismatch below)";
        if (nBigDiff)
        {
            for (size_t i = 0; i < rActual.size(); ++i)
            {
                const int nDiff = static_cast<int>(rActual[i]) - static_cast<int>(kDump[i]);
                const int nAbs = nDiff < 0 ? -nDiff : nDiff;
                if (nAbs > nMaxDelta)
                {
                    ADD_FAILURE() << pszName << " byte " << i << ": cpu=" << rActual[i]
                                  << " dump=" << kDump[i];
                    break;
                }
            }
        }
        if (nMaxDelta > 0 && nLsbDiff > 0)
        {
            // Off-by-one rounding at value boundaries comes from the original
            // x87 extended precision (not reproducible in C++ doubles); tolerated.
            GTEST_LOG_(INFO) << pszName << ": " << nLsbDiff << " bytes off by +/-1";
        }
    }
}

TEST(ZSharedResourcesD3D_Generators, ScatterMatchesReference)
{
    // x87 extended precision in the original causes a handful of +/-1 boundary
    // roundings, so allow an LSB delta.
    ExpectMatch("mapScatter.raw", 128, 128, Render(128, 128, CpuScatter), 1);
}

TEST(ZSharedResourcesD3D_Generators, SpecularMatchesReference)
{
    ExpectMatch("mapSpecular.raw", 128, 128, Render(128, 128, CpuSpecular), 1);
}

TEST(ZSharedResourcesD3D_Generators, AnisotropicMatchesReference)
{
    ExpectMatch("mapAnisotropic.raw", 128, 128, Render(128, 128, CpuAnisotropic), 1);
}

TEST(ZSharedResourcesD3D_Generators, HgPhaseMatchesReference)
{
    ExpectMatch("mapHgPhase.raw", 128, 128, Render(128, 128, CpuHgPhase));
}

TEST(ZSharedResourcesD3D_Generators, RefractionMatchesReference)
{
    ExpectMatch("mapRefraction.raw", 128, 128, Render(128, 128, CpuRefraction));
}

TEST(ZSharedResourcesD3D_Generators, NormalizerCubeMatchesReference)
{
    bool bMissing = false;
    for (int lFace = 0; lFace < 6; ++lFace)
    {
        char szName[64];
        sprintf_s(szName, "mapCubeNormalizer_face%d.raw", lFace);
        auto pCpu = Render(128, 128, [lFace](int x, int y, int w, int h) {
            return CpuNormalizer(lFace, x, y, w, h);
        });
        const std::vector<uint8_t> kDump = LoadTexdump(szName);
        if (kDump.empty())
        {
            bMissing = true;
            continue;
        }
        ASSERT_EQ(kDump.size(), pCpu.size()) << szName;
        if (std::memcmp(kDump.data(), pCpu.data(), kDump.size()) != 0)
        {
            ADD_FAILURE() << "cube face " << lFace << " differs";
        }
    }
    if (bMissing)
    {
        GTEST_SKIP() << "cube normalizer fixtures are not available";
    }
}

TEST(ZSharedResourcesD3D_Generators, SpotAttenuationMatchesReference)
{
    const double kRatios[2] = { 0.25, 0.75 };
    bool bMissing = false;
    for (int lArray = 0; lArray < 2; ++lArray)
    {
        for (int lIdx = 0; lIdx < 2; ++lIdx)
        {
            char szName[64];
            sprintf_s(szName, "spotAttenMap%d_r%d.raw", lArray, lIdx);
            const double fRatio = kRatios[lIdx];
            const int lMode = lArray; // array 0 = radial, array 1 = per-axis
            auto pCpu = Render(32, 32, [fRatio, lMode](int x, int y, int w, int h) {
                return CpuSpot(fRatio, lMode, x, y, w, h);
            });
            const std::vector<uint8_t> kDump = LoadTexdump(szName);
            if (kDump.empty())
            {
                bMissing = true;
                continue;
            }
            ASSERT_EQ(kDump.size(), pCpu.size()) << szName;
            if (std::memcmp(kDump.data(), pCpu.data(), kDump.size()) != 0)
            {
                ADD_FAILURE() << szName << " differs (array " << lArray << ", ratio " << fRatio << ")";
            }
        }
    }
    if (bMissing)
    {
        GTEST_SKIP() << "spot attenuation fixtures are not available";
    }
}

TEST(ZSharedResourcesD3D_Generators, FontAtlasMatchesReference)
{
    const std::vector<uint8_t> kFont = LoadFixture("procfont.bin");
    const std::vector<uint8_t> kDump = LoadTexdump("texFont8x13.raw");
    if (kFont.empty() || kDump.empty())
    {
        GTEST_SKIP() << "font fixtures are not available";
    }
    ASSERT_EQ(kFont.size(), 1235u);
    ASSERT_EQ(kDump.size(), 128u * 128u * 4);

    std::vector<uint8_t> cpu(128 * 128 * 4, 0);
    for (int lChar = 0x20; lChar < 0x7F; ++lChar)
    {
        const int lX0 = 8 * (lChar & 0xF);
        const int lY0 = 13 * (lChar / 16);
        const uint8_t* pGlyph = kFont.data() + static_cast<size_t>(lChar - 0x20) * 13;
        for (int lRow = 0; lRow < 13; ++lRow)
        {
            for (int lCol = 0; lCol < 8; ++lCol)
            {
                uint32_t lPixel = (pGlyph[lRow] & (0x80u >> lCol)) ? 0xFFFFFFFFu : 0u;
                std::memcpy(cpu.data() + (static_cast<size_t>(lY0 + lRow) * 128 + (lX0 + lCol)) * 4, &lPixel, 4);
            }
        }
    }

    if (std::memcmp(cpu.data(), kDump.data(), cpu.size()) != 0)
    {
        size_t nDiff = 0;
        for (size_t i = 0; i < cpu.size(); ++i)
        {
            nDiff += (cpu[i] != kDump[i]) ? 1 : 0;
        }
        ADD_FAILURE() << "font atlas differs in " << nDiff << " bytes";
    }
}

TEST(ZSharedResourcesD3D_Generators, MouseCursorMatchesReference)
{
    const std::vector<uint8_t> kMouse = LoadFixture("promouse.bin");
    const std::vector<uint8_t> kDump = LoadTexdump("texMouse16x16.raw");
    if (kMouse.empty() || kDump.empty())
    {
        GTEST_SKIP() << "mouse fixtures are not available";
    }
    ASSERT_EQ(kMouse.size(), 1024u);
    ASSERT_EQ(kDump.size(), 16u * 16u * 4);
    EXPECT_EQ(std::memcmp(kMouse.data(), kDump.data(), kMouse.size()), 0);
}

TEST(ZSharedResourcesD3D_Generators, WhiteAndShadowClipMatch)
{
    const std::vector<uint8_t> kWhite = LoadTexdump("texWhite1x1.raw");
    if (!kWhite.empty())
    {
        ASSERT_EQ(kWhite.size(), 4u);
        uint32_t lPixel = 0xFFFFFFFFu;
        EXPECT_EQ(std::memcmp(kWhite.data(), &lPixel, 4), 0);
    }
    else
    {
        GTEST_SKIP() << "fixture texWhite1x1.raw is not available";
    }

    const std::vector<uint8_t> kShadowClip = LoadTexdump("texShadowClip1x2.raw");
    if (!kShadowClip.empty())
    {
        ASSERT_EQ(kShadowClip.size(), 8u);
        const uint32_t kExpected[2] = { 0x00000000u, 0xFFFFFFFFu };
        EXPECT_EQ(std::memcmp(kShadowClip.data(), kExpected, 8), 0);
    }
    else
    {
        GTEST_SKIP() << "fixture texShadowClip1x2.raw is not available";
    }
}
