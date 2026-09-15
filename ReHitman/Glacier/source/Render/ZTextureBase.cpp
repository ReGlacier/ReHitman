#include <Glacier/Render/ZTextureBase.h>
#include <cstring>


namespace Glacier
{
    ZTextureBase::ZTextureBase() = default;

    void ZTextureBase::SetName(const char* pszName)
    {
        if (!pszName) return;

        size_t len = strlen(pszName);

        if (len >= sizeof(m_szName))
        {
            pszName += len - (sizeof(m_szName) - 1);
        }

        strncpy(m_szName, pszName, sizeof(m_szName) - 1);
        m_szName[sizeof(m_szName) - 1] = '\0';
    }
}