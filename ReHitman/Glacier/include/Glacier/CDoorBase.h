#pragma once

#include <Glacier/ZOpeningBase.h>

namespace Glacier
{
    class CDoorBase : public ZOpeningBase
    {
    public:
        // vftable
        virtual void SetPercentOpen(float);
        virtual void __pure_function_0(); //Pure virtual
        virtual void __pure_function_1(); //Pure virtual
        virtual void PostInit2();
        virtual void LoadObject(Glacier::IInputSerializerStream&);
        virtual void SaveObject(Glacier::IOutputSerializerStream&);

        //TODO: Complete later, it's too hard for now
    };
}