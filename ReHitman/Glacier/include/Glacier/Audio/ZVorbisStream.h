#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier
{
    class ZVorbisStream
    {
    public:
        // vtbl
        virtual ~ZVorbisStream();
        virtual bool Init();
        virtual void Run(int, int&);
        virtual void End();

        // methods

        // members
    };
    // Possible size 0x2328
}
