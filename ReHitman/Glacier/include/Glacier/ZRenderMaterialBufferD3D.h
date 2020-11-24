#pragma once

#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    class ZRenderMaterialBufferD3D
    {
    public:
        // VFTable
        virtual void Release(bool); //+0x0
        virtual void Function_1(int a0); //+0x4
        virtual void Function_2(int a0, int a1, int a2); //+0x8
        virtual void Function_3(); //+0xC Free +0x4, erase 0x8, 0xC, 0x10
        virtual void Function_4(); //+0x10 Require 0x4 inited, take byte from 0x40AC, if that byte is zero - call sub_494370 and set it to 1
        virtual void Function_5(); //+0x14 if 40AC inited -> init dword_90AF20 (???)
        virtual int  Function_6(int a2); //+0x18 return this + (4 * a2) + 0x20AC, some value by index from buffer
        /*
         * @fn Function_7
         * @brief
         *      Called from 0x00476599
         *      Iterates over buffer from 0x20AC, 0x2AC - buffer of pointers
         *      Each pointer points to entry
         *      Breaks if
         *          int* buf = (int*)0x20AC;
         *          v6 = buf[i]
         *          if (v6->field58 == 2 && v6->field5A == 13)
         *              break;
         *      Total 0x800 entries in 0x20AC buffer
         * @return Index of entry where look was stopped
         * @param a1 - not used
         * @param a2 - not used
         * @param a3 - ???
         */
        virtual unsigned int Function_7(int a1, int a2, int a3);
        /*
         * @fn Function_8
         * @param a1 - passed to Function_7 (no other args usage, need to understand wtf)
         * @param a2 - pointer to string, compares with entries
         */
        virtual int Function_8(int a1, const char* a2); //+0x1C
        /*
         * @brief Fill array passed by pointer through a2
         * @param a1 - passed to Function_7
         * @param a2 - pointer to array (?)
         * @param a3 - size of a2
         */
        virtual int Function_9(int a1, int* arr, int sizeOfArr); //+0x20
        /**
         * @brief iterates over something, interact with g_pD3DDll
         * @param a1 - passed to unknown function, ?
         * @return minimum value of smth(?)
         */
        virtual unsigned int Function_10(int a1); //+0x24
        virtual int GetField20A8(); //+0x28

        // Data
        char m_field4[8356];
        int m_field20A8;
        int *m_field20AC[2048];
        int m_field40AC;
        int m_field40B0;
    }; //size 0x40B0
}