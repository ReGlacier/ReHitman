#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/ZDrawBuffer.h>
#include <Glacier/ZCameraSpace.h>

namespace Hitman::BloodMoney
{
    using SPRITETYPE = int;

    class ZWINOBJ : public Glacier::ZSTDOBJ {
    public:
        // vftable
        virtual void CreateSpriteArray(uint32_t, uint32_t, SPRITETYPE);
        virtual void FreeSpriteArray(uint32_t);
        virtual void DrawRaw(Glacier::ZDrawBuffer*, float const*, float const*, float const*, float);
        virtual void Draw(Glacier::ZDrawBuffer *, Glacier::ZCameraSpace*, float const*, float const*);
        virtual void Draw(Glacier::ZDrawBuffer*, Glacier::ZCameraSpace*, Glacier::ZMat3x3* transform, Glacier::Vector3* position, unsigned char);
        virtual void RecalcMaxMin();
        virtual void GetMouseColi(Glacier::SMouseColi*, float*, float*);
        virtual void RemoveGeometry();
        virtual void LoadSaveGeometry(Glacier::ZPackedInput*, bool);
        virtual void SetDrawMode(uint32_t, int);
        /**
         * a2 table:
         * 1 -> 0x16C012;
         * 2 -> fault
         * 3: -> 0x16C011;
         * 4 -> fault
         * 5 -> 0x16C210
         * 6 -> 0x16C211
         * other -> 0x16C010
         */
        virtual int32_t CalcDrawMode(int a2, int a3);
        virtual void SetDrawing();
        virtual void SetColor(uint32_t);
        virtual void SetColor(uint32_t, bool);
        virtual void SetColor(Glacier::Vector3* pColor); // Vec3F or other
        virtual void SetAlpha(uint8_t);
        virtual void SetScale(Glacier::Vector2* scale, bool /* unused */);
        virtual void SetAlignment(uint8_t alignType);
        virtual uint8_t GetAlignment();
        virtual void SetPos(float, float, float);
        virtual void SetType(uint8_t);
        virtual uint32_t GetDrawMode();
        virtual void SetPriority(uint8_t);

        // api

        // data (total size is 0x88, base size is 0x10)
        int m_field10; //+0x10
        int m_field14; //+0x14
        int m_field18; //+0x18
        int m_field1C; //+0x1C
        int m_field20; //+0x20
        int m_field24; //+0x24
        int m_field28; //+0x28
        int m_field2C; //+0x2C
        int m_field30; //+0x30
        int m_field34; //+0x34
        int m_field38; //+0x38
        int m_field3C; //+0x3C
        int m_field40; //+0x40
        int m_field44; //+0x44
        int m_field48; //+0x48
        int m_field4C; //+0x4C
        int m_field50; //+0x50
        int m_field54; //+0x54
        int m_field58; //+0x58
        int m_field5C; //+0x5C
        int m_field60; //+0x60
        int m_field64; //+0x64
        int m_field68; //+0x68
        int m_field6C; //+0x6C
        int m_field70; //+0x70
        int m_field74; //+0x74
        int m_field78; //+0x78
        int m_field7C; //+0x7C
        int m_field80; //+0x80
        int m_field84; //+0x84
    };
}