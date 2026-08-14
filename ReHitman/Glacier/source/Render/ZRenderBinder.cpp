#include <Glacier/Render/ZRenderBinder.h>
#include <cstring>


namespace Glacier
{
    ZRenderBinder::~ZRenderBinder() = default;
    
    void ZRenderBinder::PrintInfo()
    {
        // Do nothing
    }

    bool ZRenderBinder::IsParameterUsed(const char* pszParameter)
    {
        return strcmp(m_pszName, pszParameter) == 0;
    }
}