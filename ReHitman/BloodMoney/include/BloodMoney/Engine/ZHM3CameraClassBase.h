#pragma once

#include <Glacier/ZCAMERA.h>

namespace Hitman::BloodMoney
{
    class ZHM3CameraClassBase : public Glacier::ZCAMERA
    {
    public:

        //data (total size is 0x1A8, ZCAMERA size is 0x18C)
        int m_field18C;
        int m_field190;
        int m_field194;
        int m_field198;
        int m_field19C;
        int m_field1A0;
        int m_field1A4;
    };
}