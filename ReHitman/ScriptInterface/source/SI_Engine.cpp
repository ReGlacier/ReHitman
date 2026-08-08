#include <SI/SI_Engine.h>

#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ResourceCollection.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZUniAssert.h>

#include <cstdio>
#include <cmath>


namespace Glacier
{
    void Print(const char* format, ...)
    {
        // TODO: Re-implement IOI formatter? See PS2 IOIformat method
    }

    void Error(const char* format, ...)
    {
        // TODO: Finish me
    }

    void Warning(const char* format, ...)
    {
        // TODO: Finish me
    }

    void Zscassert(bool bCond)
    {
        if (!bCond)
        {
            printf("[SI_Engine.cpp] ASSERT: Script condition failed\n");
            ZASSERT(false);
        }
    }

    float Engine__Random()
    {
        return g_pSysInterface->FRand(nullptr, 0);
    }

    int Engine__Randomrange(int min, int max)
    {
        return static_cast<int>(g_pSysInterface->FRand(nullptr, 0)) * (max - min) + min;
    }

    float Engine__Gettime()
    {
        return static_cast<float>(g_pSysInterface->FrameTime);
    }

    ZREF Engine__Getsceneval(const char* name)
    {
        // TODO: Finish me
        return 0;
    }

    void Engine__Setsceneval(const char* name, ZREF value)
    {
        // TODO: Finish me
    }

    bool Engine__Localeexists(const char* name)
    {
        ZASSERT(g_pEngineData);(g_pEngineData);

        return g_pEngineData->m_pLocaleResources->HasResource(name);
    }

    bool Engine__Isvalidvector(float x, float y, float z)
    {
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
    }
}
