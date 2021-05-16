#pragma once

#include <cstdint>

namespace Glacier
{
    /**
     * @brief Really don't understand for what but in some cases field 18, 1C are used.
     */
    struct SUnknown1 {
        float m_field0; // Some percents? (multiplied by 100.f in sub_477D50)
        float m_field4;
        float m_field8;
        float m_fieldC;
        float m_field10;
        float m_field14; // Multiplied by field0 & 100.f
        int m_field18; // used
        int m_field1C; // used
        //0x40 - Render View
        //0x64 - [ptr] some another ZRenderEntry?
    };

    class ZRenderEntry
    {
    public:
        //vftable
        virtual void Release(bool freeMem);
        virtual int Unknown1(SUnknown1* pUnknown);
        virtual void Unknown2();
        virtual void Unknown3();
        // Not completed

        //data
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
        int m_field18;
        int m_field1C;
        int m_field20;
        int m_field24;
        char m_field28;
        bool m_field29;
        char m_field2A;
        char m_field2B;
        int m_field2C;
        uint16_t m_field30; // Some mask?
        uint16_t m_field32; // Some counter? (iterates at sub_476EE0)
        int m_field34;
        int m_field38;
        int m_field3C;
        int m_field40;
        int m_field44;
    };
}