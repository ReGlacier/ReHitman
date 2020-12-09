#pragma once

#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    class ZOpeningBase
    {
    public:
        // vftable
        virtual void* FindReferenceNormal();
        virtual void FindRootNormal(); // pure virtual
        virtual void* FindGates();
        virtual Glacier::ZGEOM* GetBaseGeom(); // pure virtual
        virtual void AdjustOpenness(float, float);
        virtual void ZOpeningBase_Destructor();
        virtual void Init();
        virtual void Init2();
    };
}