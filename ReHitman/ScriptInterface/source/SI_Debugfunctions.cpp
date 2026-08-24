#include <Glacier/Component/Debug/ZDrawDebugObjectComponent.h>
#include <SI/SI_Debugfunctions.h>


namespace Glacier
{
    // DronCode: 100% of that functions are empty in PC build
    //           But I'd like to port all that stuff from PS2
    //           Yep, I know that it's unused in PC final DLLs, but I don't care

    void Addlinedecay(v3 start, v3 end, int color, float decay)
    {
        // TODO: Finish me
    }

    void Addvectordecay(v3 start, v3 end, float decay, int color, float arrowLength)
    {
        // TODO: Finish me
    }

    void Debugfunctions__Displayscriptline(ZREF firstGeom, ZREF secondGeom, int color)
    {
        // TODO: Finish me
    }

    void Debugfunctions__Displayscriptlinefrompos(v3 start, v3 end, int color)
    {
        // TODO: Finish me
    }

    int Debugfunctions__Displaypermanentscriptline(ZREF firstGeom, ZREF secondGeom, int color)
    {
        // TODO: Finish me
        return 0;
    }

    int Debugfunctions__Displaypermanentscriptlinefrompos(v3 start, v3 end, int color)
    {
        // TODO: Finish me
        return 0;
    }

    bool Debugfunctions__Modifypermanentscriptline(int line, ZREF firstGeom, ZREF secondGeom, int color)
    {
        // TODO: Finish me
        return false;
    }

    int Debugfunctions__Addpermanentscripttext(ZREF geom, int height, int color, const char* format, ...)
    {
        // TODO: Finish me
        return 0;
    }
}
