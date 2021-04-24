#pragma once

namespace Glacier
{
    class ZRenderObject
    {
    public:
        //vftable
        virtual void Release(bool doFreeMem);
        virtual void* CreateInstance();
        virtual int DoSmth(); ///???

        //data
    };
}