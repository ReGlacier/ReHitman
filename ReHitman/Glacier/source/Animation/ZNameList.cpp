#include <Glacier/Animation/ZNameList.h>
#include <cstring>


namespace Glacier::Animation
{
    int ZNameList::GetId(const char* pszAnimName, int iNoneIndex)
    {
        if (!m_Names || !pszAnimName)
            return iNoneIndex;

        const char* currentStr = m_Names;
        const char* endOfBuffer = &m_Names[m_Size];
        int currentId = 0;

        while (currentStr < endOfBuffer && *currentStr)
        {
            if (stricmp(pszAnimName, currentStr) == 0)
            {
                return currentId;
            }

            while (*currentStr)
            {
                ++currentStr;
            }

            ++currentStr;
            ++currentId;
        }

        return iNoneIndex;
    }

    const char* ZNameList::GetName(int id, int none)
    {
        if (!m_Names || id == none || id >= m_Count)
            return nullptr;

        const char* currentStr = m_Names;
        const char* endOfBuffer = &m_Names[m_Size];

        if (id > 0)
        {
            do
            {
                if (currentStr >= endOfBuffer)
                    break;

                while (*currentStr)
                {
                    ++currentStr;
                }

                --id;
                ++currentStr;
            }
            while (id > 0);
        }

        if (id == 0 && currentStr < endOfBuffer && *currentStr)
        {
            return currentStr;
        }

        return nullptr;
    }
} // namespace Glacier::Animation
