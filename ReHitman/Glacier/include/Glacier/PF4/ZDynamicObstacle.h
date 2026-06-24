#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/PF4/ZObstacle.h>

namespace Glacier::PF4
{
	struct ZDynamicObstacle : public ZObstacle
	{
        struct ZLink
        {
            struct ZLink* m_Next;
            struct ZLink* m_Prev;
            struct ZLink* m_This;
        };

        float m_ConvexHull[10][2];      //+0x00  (10 points * 2 floats * 4 bytes = 80 bytes)
        int m_HullSize;                 //+0x50  (bit 640)
        int m_Type;                     //+0x54  (bit 672)
        float m_Radius;                 //+0x58  (bit 704)
        float m_Dir[2];                 //+0x5C  (bit 736, 64 bits = 8 bytes)
        float m_Location[5];            //+0x64  (bit 800, 160 bits = 20 bytes)
        ZLink m_Linking[16];            //+0x78  (bit 960, 16 links * 12 bytes = 192 bytes)
        char m_Component[64];           //+0x138 (bit 2496, 512 bits = 64 bytes)
        int m_iLinks;                   //+0x178 (bit 3008)
        // Total structure size: 380 bytes (0x17C)
	};
    RE_VERIFY_SIZE(ZDynamicObstacle, 0x17C);
}