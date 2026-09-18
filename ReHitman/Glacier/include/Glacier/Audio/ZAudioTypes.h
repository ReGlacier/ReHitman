#pragma once


namespace Glacier
{
    class ZAudioTypes
    {
    public:
        // types
        enum EType
        {
            Base = 0x0,
            Sound = 0x1,
            Set = 0x2,
            Seq = 0x3,
            Wave = 0x4,
            Def = 0x5,
            DefMaster = 0x6,
            RoomEffect = 0x7,
            LowPass = 0x8,
            Ambience = 0x9,
            LocationSounds = 0xA,
            LocationCues = 0xB,
            Location = 0xC,
            Acoustics = 0xE,
            Ambience3d = 0xF,
            Dropped = 0x10,
            Mapping = 0x11,
            ObjectProperty = 0x12,
            SurfaceProperty = 0x13,
            Material = 0x14,
            Filter = 0x15,
            WaveHeader = 0x16,
            Group = 0x17,
            Layered = 0x18,
        };
    };
}
