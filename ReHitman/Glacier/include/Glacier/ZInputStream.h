#pragma once

#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    class ZIOInputStream;

    class ZInputStream
    {
    public:
        // data
        int field_4;
        ZIOInputStream* m_IOInputStream;
        int field_C;
        int field_10;
        int field_14;
        int field_18;
        int field_1C;
        int field_20;
        int field_24;
        int field_28;
        int field_2C;
        int field_30;
        int field_34;
        int field_38;
        int field_3C;
        int field_40;
        int field_44;
        int field_48;
        int field_4C;
        int field_50;

        // vftable
        virtual ~ZInputStream();

        /**
         * @fn ReadRaw
         * @param buffer destination buffer
         * @param size size of destination buffer or how much bytes should be stored inside from original buffer
         * @return count of read bytes
         */
        virtual unsigned int ReadRaw(char* buffer, unsigned int size);

        /**
         * @fn ReadChangeEndianness
         * @param buffer
         * @param size
         * @param a3
         */
        virtual void ReadChangeEndianness(char* buffer, unsigned int size, unsigned int a3);
    };
}