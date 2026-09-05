#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB.h>


namespace Glacier
{
    /**
     * @class REFTAB32
     * @brief A specialized, fixed-capacity variant of @ref REFTAB optimized for up to 32 elements.
     *
     * @details
     * REFTAB32 extends the base @ref REFTAB container by embedding the **very first data block** * directly inside the class instance memory layout layout (Small Buffer Optimization / SBO).
     * * ### Internal Memory & Math Breakdown:
     * - The class reserves a local byte array `m_FirstTab` of exactly **144 bytes**.
     * - **144 bytes** corresponds to: `sizeof(TabBlk)` (16 bytes) + `32 elements * sizeof(uint32_t)` (128 bytes).
     * - Total structure size strictly evaluates to **0xAC** (172 bytes), preserving flawless 1:1 binary alignment 
     * with the original Glacier Engine.
     *
     * ### Performance Strategy:
     * 1. **Zero-Allocation Setup:** For short-lived or small collections (<= 32 elements), this container performs 
     * **no dynamic heap allocations** whatsoever. It recycles the pre-allocated stack/embedded `m_FirstTab` buffer.
     * 2. **Fallback to Chaining:** If the 33rd element is added, @ref NewBlock overrides seamlessly kick in, triggering 
     * the engine's heap allocator (`ZNetAlloc::Allocate`) to chain subsequent blocks into a standard linked list.
     * 3. **Destruction Safety:** @ref DeleteBlock ensures that the embedded `m_FirstTab` region is skipped during 
     * heap cleanup routines, eliminating any risks of dangling pointers or `free()` corruption.
     */
    class REFTAB32 : public REFTAB
    {
    public:
        /**
         * @brief Standard constructor initializing the underlying REFTAB with a hardcoded 
         * pool size of 32 elements.
         */
        REFTAB32();

        /**
         * @brief Call of REFTAB' constructo IS PROHIBITED due data layout limitations
         */
        REFTAB32(int, int) = delete;

        // Our "custom" dtor
        ~REFTAB32() override;

        // vtbl (dtor not overloaded due PC impl refs to original dtor)
        /**
         * @brief Overrides memory allocation. Returns a pointer to @ref m_FirstTab on the first call,
         * or falls back to system allocation if additional chained blocks are required.
         */
        void DeleteBlock(TabBlk *) override;

        /**
         * @brief Overrides block deallocation. Safely ignores requests to free @ref m_FirstTab,
         * while calling standard release operators on any dynamically allocated secondary blocks.
         */
        TabBlk* NewBlock(void) override;
        
        // members
        /**
         * @brief Inlined memory block acting as the first chunk for up to 32 elements.
         * @note Combines 16 bytes for @ref TabBlk header and 128 bytes for 32 x uint32_t elements.
         */
        char m_FirstTab[144];
    };
    RE_VERIFY_SIZE(REFTAB32, 0xAC);

    template <typename T> T* get(REFTAB32* reftab, size_t index)
    {
        return reftab ? reftab->operator[](index) : nullptr;
    }
}