#pragma once

namespace Glacier
{

    class ZQElemsBuffer
    {
    public:
        // data
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;

        // vftable
        virtual void Release();
    };
}