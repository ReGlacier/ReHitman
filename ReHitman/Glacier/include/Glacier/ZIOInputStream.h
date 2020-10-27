#pragma once

namespace Glacier
{
    class ZIOInputStream
    {
    public:
        // vftable
        virtual void Func0();

        // data
        void* m_buffer;
        int m_size;
    };
}