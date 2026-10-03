#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    /**
     * @brief Collects candidate strings for console TAB-completion
     * (engine/zstdlib/consolecommand.cpp).
     *
     * Strings are copied into a fixed-size byte buffer from the front while the
     * pointer table grows from the back, so a single allocation serves both.
     * Only strings that start with the currently set Prefix() are accepted by Add().
     */
    class ZCompletionList
    {
    public:
        // methods
        ZCompletionList(int iBufferSize);
        ~ZCompletionList();

        /** @brief Appends @p pszString when it starts with the active prefix. Silently drops on overflow. */
        void Add(const char* pszString);
        /** @brief Sets the prefix filter used by Add() (stores the pointer, not a copy). */
        void Prefix(char* pszPrefix);
        /** @brief Sorts the list and removes duplicate entries. */
        void Unique();
        /** @brief Sorts the entries ascending, case-insensitive (heap sort). */
        void Sort();
        /** @brief Resets the list to empty and clears the prefix. */
        void Clear();
        /** @brief Number of strings currently stored. */
        int Count();
        /** @brief Returns the string at @p iIndex. */
        char* Get(int iIndex);
        /** @brief Computes the longest common case-insensitive prefix over all entries. Empty list: nullptr/0. */
        void LongestCommonPrefix(char*& pszPrefix, int& iLength);

    private:
        /** @brief Case-insensitive less-than over two table slots. */
        bool LTH(int iLhs, int iRhs);
        /** @brief Swaps two table slots. */
        void Swap(int iLhs, int iRhs);
        /** @brief Sifts the heap node at @p iIndex down (used by Sort). */
        void Heapify(int iIndex);

    public:
        // members
        char* m_pBuffer;
        int m_iBufferSize;
        int m_iAllocPos;
        char** m_pszStrings;
        int m_iCount;
        char* m_szPrefix;
    };
    RE_VERIFY_SIZE(ZCompletionList, 0x18);
    RE_VERIFY_OFFSET(ZCompletionList, m_pBuffer, 0x00);
    RE_VERIFY_OFFSET(ZCompletionList, m_iBufferSize, 0x04);
    RE_VERIFY_OFFSET(ZCompletionList, m_iAllocPos, 0x08);
    RE_VERIFY_OFFSET(ZCompletionList, m_pszStrings, 0x0C);
    RE_VERIFY_OFFSET(ZCompletionList, m_iCount, 0x10);
    RE_VERIFY_OFFSET(ZCompletionList, m_szPrefix, 0x14);
}
