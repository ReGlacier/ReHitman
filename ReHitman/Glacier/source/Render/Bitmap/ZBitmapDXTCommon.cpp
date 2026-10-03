#include <Glacier/Render/Bitmap/ZBitmapDXTCommon.h>
#include <cstring>


namespace Glacier
{
    void Decode4x4DXT_RGB(uint32_t* pDst, const uint16_t* pBlock, int pitch)
    {
        const uint32_t c0 = pBlock[0];
        const uint32_t c1 = pBlock[1];

        const uint32_t r0 = (c0 >> 8) & 0xF8;
        const uint32_t g0 = (c0 >> 3) & 0xFC;
        const uint32_t b0 = (c0 & 0x1F) << 3;
        const uint32_t r1 = (c1 >> 8) & 0xF8;
        const uint32_t g1 = (c1 >> 3) & 0xFC;
        const uint32_t b1 = (c1 & 0x1F) << 3;

        uint32_t palette[4];
        palette[0] = 0xFF000000u | (r0 << 16) | (g0 << 8) | b0;
        palette[1] = 0xFF000000u | (r1 << 16) | (g1 << 8) | b1;
        if (c0 <= c1)
        {
            palette[2] = 0xFF000000u | (((r0 + r1) >> 1) << 16) | (((g0 + g1) >> 1) << 8) | ((b0 + b1) >> 1);
            palette[3] = 0;
        }
        else
        {
            palette[2] = 0xFF000000u | (((2 * r0 + r1) / 3) << 16) | (((2 * g0 + g1) / 3) << 8) | ((2 * b0 + b1) / 3);
            palette[3] = 0xFF000000u | (((r0 + 2 * r1) / 3) << 16) | (((g0 + 2 * g1) / 3) << 8) | ((b0 + 2 * b1) / 3);
        }

        const uint32_t indices = *reinterpret_cast<const uint32_t*>(pBlock + 2);
        for (int i = 0; i < 16; ++i)
        {
            pDst[(i & 3) + (i >> 2) * pitch] = palette[(indices >> (2 * i)) & 3];
        }
    }

    void Decode4x4DXT3(uint32_t* pDst, const uint16_t* pBlock, int pitch)
    {
        const uint32_t* pAlpha = reinterpret_cast<const uint32_t*>(pBlock);
        const uint16_t* pColor = pBlock + 4;

        const uint32_t c0 = pColor[0];
        const uint32_t c1 = pColor[1];

        const uint32_t r0 = (c0 >> 8) & 0xF8;
        const uint32_t g0 = (c0 >> 3) & 0xFC;
        const uint32_t b0 = (c0 & 0x1F) << 3;
        const uint32_t r1 = (c1 >> 8) & 0xF8;
        const uint32_t g1 = (c1 >> 3) & 0xFC;
        const uint32_t b1 = (c1 & 0x1F) << 3;

        // The color part keeps the DXT1 decoder shape; alpha comes from the alpha block.
        uint32_t palette[4];
        palette[0] = (r0 << 16) | (g0 << 8) | b0;
        palette[1] = (r1 << 16) | (g1 << 8) | b1;
        if (c0 <= c1)
        {
            palette[2] = (((r0 + r1) >> 1) << 16) | (((g0 + g1) >> 1) << 8) | ((b0 + b1) >> 1);
            palette[3] = 0;
        }
        else
        {
            palette[2] = (((2 * r0 + r1) / 3) << 16) | (((2 * g0 + g1) / 3) << 8) | ((2 * b0 + b1) / 3);
            palette[3] = (((r0 + 2 * r1) / 3) << 16) | (((g0 + 2 * g1) / 3) << 8) | ((b0 + 2 * b1) / 3);
        }

        const uint32_t indices = *reinterpret_cast<const uint32_t*>(pBlock + 6);
        for (int i = 0; i < 16; ++i)
        {
            // 4-bit alpha expanded to 8 bits (PC: 0x11000000 * nibble)
            const uint32_t alpha = ((pAlpha[i >> 3] >> (4 * (i & 7))) & 0xF) * 0x11;
            pDst[(i & 3) + (i >> 2) * pitch] = palette[(indices >> (2 * i)) & 3] + (alpha << 24);
        }
    }

    namespace
    {
        // Channel extraction shifts for R, G, B, A (PC: unk_7F4CA4)
        constexpr int kChannelShift[4] = { 0, 8, 16, 24 };
        // Endpoint inset for the range-fit sweep (PC: unk_7F4CA0)
        constexpr int kEndpointInset = 32;
        // Alpha extraction shift for DXT3 (PC: unk_7F4CC0)
        constexpr int kAlphaShift = 24;

        // Per-channel min/max over a deinterleaved 4x4 block (PC: DXTComputeMinMax)
        void DXTComputeMinMax(int count, const int* pChannels, int* pMin, int* pMax)
        {
            pMin[0] = 255; pMax[0] = 0;
            pMin[1] = 255; pMax[1] = 0;
            pMin[2] = 255; pMax[2] = 0;
            pMin[3] = 255; pMax[3] = 0;

            for (int i = 0; i < count; ++i)
            {
                for (int c = 0; c < 4; ++c)
                {
                    const int v = pChannels[4 * i + c];
                    if (v < pMin[c]) pMin[c] = v;
                    if (v > pMax[c]) pMax[c] = v;
                }
            }
        }

        // Orders R/G/B channels by descending variance (PC: DXTComputeChannelOrder)
        void DXTComputeChannelOrder(int count, const int* pChannels, int* pOrder)
        {
            float variance[3];
            for (int c = 0; c < 3; ++c)
            {
                pOrder[c] = c;
                int sum = 0;
                int sumSq = 0;
                for (int i = 0; i < count; ++i)
                {
                    const int v = pChannels[4 * i + c];
                    sum += v;
                    sumSq += v * v;
                }
                variance[c] = static_cast<float>(
                    static_cast<double>(sumSq) * (1.0 / static_cast<double>(count))
                    - static_cast<double>(sum * sum) * (1.0 / static_cast<double>(count)) * (1.0 / static_cast<double>(count)));
            }

            if (variance[0] < variance[1])
            {
                const float fv = variance[0]; variance[0] = variance[1]; variance[1] = fv;
                const int ov = pOrder[0]; pOrder[0] = pOrder[1]; pOrder[1] = ov;
            }
            if (variance[0] < variance[2])
            {
                const float fv = variance[0]; variance[0] = variance[2]; variance[2] = fv;
                const int ov = pOrder[0]; pOrder[0] = pOrder[2]; pOrder[2] = ov;
            }
            if (variance[1] < variance[2])
            {
                const int ov = pOrder[1]; pOrder[1] = pOrder[2]; pOrder[2] = ov;
            }
        }

        // Expands grid-aligned endpoints to 8 bits via bit replication (PC: DXTExpandEndpoints).
        // Note: in alpha mode the second endpoint's green channel still expands as 6-bit,
        // matching the PC code.
        void DXTExpandEndpoints(int bAlpha, const int* pEp0, const int* pEp1, int* pOut0, int* pOut1)
        {
            pOut0[0] = pEp0[0] | (pEp0[0] >> 5);
            pOut1[0] = pEp1[0] | (pEp1[0] >> 5);
            pOut0[2] = pEp0[2] | (pEp0[2] >> 5);
            pOut1[2] = pEp1[2] | (pEp1[2] >> 5);

            pOut0[1] = bAlpha ? (pEp0[1] | (pEp0[1] >> 5)) : (pEp0[1] | (pEp0[1] >> 6));
            pOut1[1] = pEp1[1] | (pEp1[1] >> 6);
        }

        // Builds the local 4-entry palette from expanded endpoints (PC: DXTBuildPalette).
        // Opaque:  [ep0, (2ep0+ep1+1)/3, (ep0+2ep1+1)/3, ep1] with A=255.
        // Alpha:   [ep0, (ep0+ep1)/2, ep1, {0,0,0,0}] with A=255 for the first three.
        void DXTBuildPalette(int bAlpha, int* pPalette, const int* pEp0, const int* pEp1)
        {
            if (bAlpha)
            {
                for (int e = 0; e < 3; ++e)
                {
                    for (int c = 0; c < 3; ++c)
                    {
                        pPalette[4 * e + c] = (e * pEp1[c] + (2 - e) * pEp0[c]) / 2;
                    }
                    pPalette[4 * e + 3] = 255;
                }
                pPalette[12] = 0;
                pPalette[13] = 0;
                pPalette[14] = 0;
                pPalette[15] = 0;
            }
            else
            {
                for (int e = 0; e < 4; ++e)
                {
                    for (int c = 0; c < 3; ++c)
                    {
                        pPalette[4 * e + c] = ((3 - e) * pEp0[c] + e * pEp1[c] + 1) / 3;
                    }
                    pPalette[4 * e + 3] = 255;
                }
            }
        }

        // Sum of per-pixel nearest-palette distances along one channel (PC: DXTSweepError).
        // Transparent pixels are measured against palette entry 3.
        int DXTSweepError(int bAlpha, int channel, const int* pEp0, const int* pEp1, const int* pBlock, int bestErr)
        {
            int ep0[3], ep1[3];
            int palette[16];
            DXTExpandEndpoints(bAlpha, pEp0, pEp1, ep0, ep1);
            DXTBuildPalette(bAlpha, palette, ep0, ep1);

            int result = 0;
            for (int i = 0; i < 16; ++i)
            {
                const int v = pBlock[4 * i + channel];
                if (pBlock[4 * i + 3])
                {
                    int best = 0x7FFFFFFF;
                    for (int e = 0; e < 4; ++e)
                    {
                        const int d = v - palette[4 * e + channel];
                        const int d2 = d * d;
                        if (d2 < best)
                        {
                            best = d2;
                        }
                    }
                    result += best;
                }
                else
                {
                    const int d = v - palette[12 + channel];
                    result += d * d;
                }
                if (result > bestErr)
                {
                    break;
                }
            }
            return result;
        }

        // Sum of per-pixel distances to the assigned palette entry along one channel (PC: DXTIndexedError)
        int DXTIndexedError(int bAlpha, int channel, const int* pEp0, const int* pEp1, const int* pBlock, const int* pIndices, int bestErr)
        {
            int ep0[3], ep1[3];
            int palette[16];
            DXTExpandEndpoints(bAlpha, pEp0, pEp1, ep0, ep1);
            DXTBuildPalette(bAlpha, palette, ep0, ep1);

            int result = 0;
            for (int i = 0; i < 16; ++i)
            {
                const int d = pBlock[4 * i + channel] - palette[4 * pIndices[i] + channel];
                result += d * d;
                if (result > bestErr)
                {
                    break;
                }
            }
            return result;
        }

        // Assigns each pixel the nearest palette entry along one channel; transparent
        // pixels get index 3 (PC: DXTAssignIndices)
        void DXTAssignIndices(int bAlpha, int channel, const int* pEp0, const int* pEp1, const int* pBlock, int* pIndices)
        {
            int ep0[3], ep1[3];
            int palette[16];
            DXTExpandEndpoints(bAlpha, pEp0, pEp1, ep0, ep1);
            DXTBuildPalette(bAlpha, palette, ep0, ep1);

            for (int i = 0; i < 16; ++i)
            {
                if (!pBlock[4 * i + 3])
                {
                    pIndices[i] = 3;
                    continue;
                }
                const int v = pBlock[4 * i + channel];
                int best = 0x7FFFFFFF;
                int index = 0;
                for (int e = 0; e < 4; ++e)
                {
                    const int d = v - palette[4 * e + channel];
                    const int d2 = d * d;
                    if (d2 < best)
                    {
                        best = d2;
                        index = e;
                    }
                }
                pIndices[i] = index;
            }
        }

        // Writes the palette colors selected by the indices back into the block (PC: DXTReconstruct)
        void DXTReconstruct(int bAlpha, const int* pEp0, const int* pEp1, int* pBlock, const int* pIndices)
        {
            int ep0[3], ep1[3];
            int palette[16];
            DXTExpandEndpoints(bAlpha, pEp0, pEp1, ep0, ep1);
            DXTBuildPalette(bAlpha, palette, ep0, ep1);

            for (int i = 0; i < 16; ++i)
            {
                for (int c = 0; c < 4; ++c)
                {
                    pBlock[4 * i + c] = palette[4 * pIndices[i] + c];
                }
            }
        }
    }

    void CompressBlockDXT1(int bAlpha, int pitch, int x, int y, const uint32_t* pSrc, uint32_t* pScratch, uint32_t* pOut)
    {
        pOut[0] = 0;
        pOut[1] = 0;

        // Deinterleave the 4x4 block into per-channel ints; the 4th slot holds the
        // opacity mask (255 for opaque, 0 for transparent).
        int block[64];
        const uint32_t* pPixel = pSrc + x + y * pitch;
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                const uint32_t px = pPixel[col];
                int* pSlot = &block[4 * (4 * row + col)];
                pSlot[0] = (px >> kChannelShift[0]) & 0xFF;
                pSlot[1] = (px >> kChannelShift[1]) & 0xFF;
                pSlot[2] = (px >> kChannelShift[2]) & 0xFF;
                pSlot[3] = static_cast<uint8_t>(px >> kChannelShift[3]) < 0x80 ? 0 : 255;
            }
            pPixel += pitch;
        }

        int minCh[4], maxCh[4];
        DXTComputeMinMax(16, block, minCh, maxCh);

        // Grid-aligned sweep bounds per channel.
        int start[3], end[3];
        if (minCh[0] == maxCh[0] && minCh[1] == maxCh[1] && minCh[2] == maxCh[2] && minCh[3] == maxCh[3])
        {
            // Solid block (including a uniform alpha mask): endpoints collapse to the single color.
            for (int k = 0; k < 3; ++k)
            {
                if (k == 1)
                {
                    start[1] = end[1] = bAlpha ? (maxCh[1] & 0xF8) : (maxCh[1] & 0xFC);
                }
                else
                {
                    start[k] = end[k] = minCh[k] & 0xF8;
                }
            }
        }
        else
        {
            for (int m = 0; m < 3; ++m)
            {
                const int grid = (m == 1 && !bAlpha) ? 0xFC : 0xF8;
                int lo = minCh[m] - kEndpointInset;
                int hi = maxCh[m] + kEndpointInset;
                if (lo < 0) lo = 0;
                if (hi > 255) hi = 255;
                start[m] = lo & grid;
                end[m] = hi & grid;
            }
        }

        // Order channels by descending variance; the first one is the master axis.
        int order[3];
        DXTComputeChannelOrder(16, block, order);
        const int master = order[0];

        int best0[3] = {};
        int best1[3] = {};
        int ep0[3] = {};
        int ep1[3] = {};
        int indices[16];

        const int step0 = (master != 1 || bAlpha) ? 8 : 4;
        const int step1 = 4 * (master != 1) + 4;

        // Sweep the master channel endpoints over the grid.
        int bestErr = 0x7FFFFFFF;
        ep0[master] = start[master];
        if (start[master] <= end[master])
        {
            do
            {
                ep1[master] = start[master];
                do
                {
                    const int err = DXTSweepError(bAlpha, master, ep0, ep1, block, bestErr);
                    if (err < bestErr)
                    {
                        bestErr = err;
                        best0[master] = ep0[master];
                        best1[master] = ep1[master];
                    }
                    ep1[master] += step1;
                }
                while (ep1[master] <= end[master]);
                ep0[master] += step0;
            }
            while (ep0[master] <= end[master]);
        }

        // Fix the master endpoints and assign per-pixel indices along it.
        ep0[master] = best0[master];
        ep1[master] = best1[master];
        DXTAssignIndices(bAlpha, master, best0, best1, block, indices);

        // Sweep the remaining two channels, refining against the assigned indices.
        // The PC code keeps the master-derived steps for both sweeps.
        for (int n = 1; n < 3; ++n)
        {
            const int ch = order[n];
            const int lo = start[ch];
            const int hi = end[ch];
            int bestChErr = 0x7FFFFFFF;
            ep0[ch] = lo;
            if (lo <= hi)
            {
                do
                {
                    ep1[ch] = start[ch];
                    do
                    {
                        const int err = DXTIndexedError(bAlpha, ch, ep0, ep1, block, indices, bestChErr);
                        if (err < bestChErr)
                        {
                            bestChErr = err;
                            best0[ch] = ep0[ch];
                            best1[ch] = ep1[ch];
                        }
                        ep1[ch] += step1;
                    }
                    while (ep1[ch] <= hi);
                    ep0[ch] += step0;
                }
                while (ep0[ch] <= hi);
            }
        }

        // (The PC build calls DXTReconstruct once more here; its result is overwritten
        //  by the final call below and never read in between, so it is omitted.)

        // Pack endpoints to 565 and order them for the target mode:
        // opaque requires c0 > c1, alpha mode requires c0 <= c1.
        const uint32_t packed0 = (static_cast<uint32_t>(best0[2]) >> 3) + 8u * ((static_cast<uint32_t>(best0[1]) & 0xFC) + 32u * (static_cast<uint32_t>(best0[0]) & 0xF8));
        const uint32_t packed1 = (static_cast<uint32_t>(best1[2]) >> 3) + 8u * ((static_cast<uint32_t>(best1[1]) & 0xFC) + 32u * (static_cast<uint32_t>(best1[0]) & 0xF8));
        const bool bSwap = bAlpha ? (packed1 < packed0) : (packed0 < packed1);
        if (bSwap)
        {
            for (int c = 0; c < 3; ++c)
            {
                const int t = best0[c]; best0[c] = best1[c]; best1[c] = t;
            }
            if (!bAlpha)
            {
                for (int i = 0; i < 16; ++i)
                {
                    indices[i] = 3 - indices[i];
                }
            }
            else
            {
                const int remap[4] = { 2, 1, 0, 3 };
                for (int i = 0; i < 16; ++i)
                {
                    indices[i] = remap[indices[i]];
                }
            }
        }

        // Remap the local palette order to the hardware order.
        {
            const int remapOpaque[4] = { 0, 2, 3, 1 };
            const int remapAlpha[4] = { 0, 2, 1, 3 };
            const int* pRemap = bAlpha ? remapAlpha : remapOpaque;
            for (int i = 0; i < 16; ++i)
            {
                indices[i] = pRemap[indices[i]];
            }
        }

        // Pack the 565 endpoints.
        pOut[0] = (static_cast<uint32_t>(best0[2]) >> 3)
                | ((static_cast<uint32_t>(best0[1]) >> 2) << 5)
                | ((static_cast<uint32_t>(best0[0]) >> 3) << 11)
                | ((static_cast<uint32_t>(best1[2]) >> 3) << 16)
                | ((static_cast<uint32_t>(best1[1]) >> 2) << 21)
                | ((static_cast<uint32_t>(best1[0]) >> 3) << 27);

        // Rebuild the block with the final indices and emit the reconstructed
        // pixels to the scratch buffer (callers never read it on PC).
        DXTReconstruct(bAlpha, best0, best1, block, indices);

        uint32_t* pScratchPixel = pScratch + x + y * pitch;
        for (int i = 0; i < 16; ++i)
        {
            pScratchPixel[(i >> 2) * pitch + (i & 3)] =
                  static_cast<uint32_t>(block[4 * i + 0])
                | (static_cast<uint32_t>(block[4 * i + 1]) << 8)
                | (static_cast<uint32_t>(block[4 * i + 2]) << 16)
                | (static_cast<uint32_t>(block[4 * i + 3]) << 24);
            pOut[1] |= static_cast<uint32_t>(indices[i]) << (2 * i);
        }
    }

    void CompressBlockDXT3(int bAlpha, int pitch, int x, int y, const uint32_t* pSrc, uint32_t* pScratch, uint32_t* pOut)
    {
        // Color part goes after the 8-byte alpha block.
        CompressBlockDXT1(bAlpha, pitch, x, y, pSrc, pScratch, pOut + 2);

        // Extract 4-bit alpha per pixel and pack it into the first 8 bytes.
        int alpha[16];
        const uint32_t* pPixel = pSrc + x + y * pitch;
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                alpha[4 * row + col] = (pPixel[col] >> kAlphaShift) >> 4;
            }
            pPixel += pitch;
        }
        pOut[0] = 0;
        pOut[1] = 0;
        for (int i = 0; i < 16; ++i)
        {
            pOut[i >> 3] |= static_cast<uint32_t>(alpha[i]) << (4 * (i & 7));
        }
    }
}
