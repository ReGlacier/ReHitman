#include <SI/SI_Istring.h>
#include <Glacier/Geom/ZGEOM.h>
#include <cstring>
#include <string_view>
#include <algorithm>


namespace Glacier
{
    namespace
    {
        bool CompareCaseInsensetive(std::string_view haystack, std::string_view needle) 
        {
            if (needle.empty()) 
                return true;
            
            if (haystack.size() < needle.size()) 
                return false;

            auto it = std::search(
                haystack.begin(), haystack.end(),
                needle.begin(), needle.end(),
                [](char a, char b) 
                {
                    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
                }
            );

            return it != haystack.end();
        }
    }

    bool Istring__Isnameequal(ZREF geom, const char* name)
    {
        auto* pGeom = ZGEOM::RefToPtr(geom);
        if (!pGeom)
            return false;

        const char* pszGeomName = pGeom->Name();
        if (!pszGeomName)
        {
            pszGeomName = "<NONAME>";
        }

        return stricmp(pszGeomName, name) == 0;
    }

    bool Istring__Isnamecontains(ZREF geom, const char* name)
    {
        auto* pGeom = ZGEOM::RefToPtr(geom);
        if (!pGeom)
            return false;

        const char* pszGeomName = pGeom->Name();
        if (!pszGeomName)
        {
            pszGeomName = "<NONAME>";
        }

        return CompareCaseInsensetive(name, pszGeomName);
    }

    bool Istring__Isempty(const char* value)
    {
        return !value || !strlen(value);
        // or
        // return !value || !value[0];
    }
}
