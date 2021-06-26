#pragma once

#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Engine/ZDirect3DDevice.h>
#include <d3d9.h>

namespace Glacier
{
    class STextureDef
    {
    public:
        int32_t m_orgPtr; //0x0000
        int32_t m_field4; //0x0004
        int32_t m_field8; //0x0008
        int32_t m_fieldC; //0x000C
        int32_t m_field10; //0x0010
        char m_name[24]; //0x0014
        IDirect3DTexture9* m_pD3DTexture; //0x002C
        int32_t m_field30; //0x0030
        int32_t m_field34; //0x0034
        int32_t m_field38; //0x0038
        int32_t m_type; //0x003C
        int32_t m_dimension; //0x0040
        int32_t m_align_44; //0x0044
    }; //Size: 0x0048

    class ZBitmap;

    class ZTextureManagerD3D
    {
    public:
        // const
        static constexpr size_t kLocalBufferSize = 0x22000;

        // vftable
        virtual ~ZTextureManagerD3D();
        virtual STextureDef* GetTexture(unsigned int index, unsigned int a2);
        virtual void LoadTEXBuffer(void* TEXBuffer);
        virtual void ReleaseTextures();
        virtual void DecodeBitmap(ZBitmap** pBitmap, char* outBuffer);
        virtual void DecodeBitmapPtr(ZBitmap* pBitmap, char* outBuffer);
        virtual unsigned int GetFreeTextureSlotsCount();
        virtual int Function_07(int, int); //?
        virtual D3DFORMAT GetNearestSupportedD3DTextureFormatByDevice(D3DFORMAT textureFormat);
        virtual bool IsSupported_V8U8_Or_L6V5U5_Or_X8L8V8U8() const;
        virtual bool IsSupported_L6V5U5_Or_X8L8V8U8() const;
        virtual bool IsSupported_DXT1() const;
        virtual bool IsDeviceEntityInitialised(uint8_t instanceIndex); //instanceIndex > 1 (!!!)

        // data
        char m_buffer[kLocalBufferSize]; //0x4
        Hitman::BloodMoney::Engine::ZDirect3DDevice* m_device; //0x22004
        int field_22008;
        int field_2200C;
        int field_22010;
        int field_22014;
        int field_22018;
        int field_2201C;
        int field_22020;
        int field_22024;
        int field_22028;
        int field_2202C;
        int field_22030;
        int m_maxTextureWidth;
        int m_maxTextureHeight;
        int field_2203C;
    };
}