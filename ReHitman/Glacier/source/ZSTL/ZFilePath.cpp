#include <Glacier/ZSTL/ZFilePath.h>
#include <Glacier/ZUniAssert.h>

#include <cstring>

namespace Glacier
{
    ZFilePath::ZFilePath()
    {
        SetValue("");
    }

    ZFilePath::ZFilePath(const char* pPath)
    {
        ZASSERT(pPath);
        SetValue(pPath);
    }

    ZFilePath::ZFilePath(const ZFilePath& copy)
    {
        std::memcpy(this, &copy, sizeof(ZFilePath));
    }

    ZFilePath& ZFilePath::operator=(const ZFilePath& copy)
    {
        if (this != &copy)
            std::memcpy(this, &copy, sizeof(ZFilePath));

        return *this;
    }

    void ZFilePath::Clear()
    {
        m_Buffer[0] = '\0';
        m_Length = 0;
    }

    void ZFilePath::Append(const ZFilePath& path)
    {
        if (path.m_Length == 0)
            return;

        if (m_Length != 0 && m_Buffer[m_Length - 1] != '/')
            m_Buffer[m_Length++] = '/';

        const char* src = path.m_Buffer;
        uint32_t srcLength = path.m_Length;

        if (srcLength != 0 && src[0] == '/')
        {
            ++src;
            --srcLength;
        }

        std::memcpy(m_Buffer + m_Length, src, srcLength + 1);
        m_Length += srcLength;
    }

    const char* ZFilePath::AsChar() const
    {
        return m_Buffer;
    }

    uint32_t ZFilePath::CharCount() const
    {
        return m_Length;
    }

    bool ZFilePath::IsEmpty() const
    {
        return CharCount() == 0;
    }

    void ZFilePath::SetValue(const char* pPath)
    {
        uint32_t length = 0;
        while (*pPath)
        {
            const char c = *pPath++;
            m_Buffer[length++] = (c == '\\') ? '/' : c;
        }

        m_Buffer[length] = '\0';
        m_Length = length;
    }
}
