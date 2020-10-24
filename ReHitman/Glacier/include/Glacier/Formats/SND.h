#pragma once

namespace Glacier::Formats
{
    using i64 = long long;
    using i32 = int;
    using i16 = short;
    using i8 = char;

    using u64 = unsigned long long;
    using u32 = unsigned int;
    using u16 = unsigned short;
    using u8 = unsigned char;

    struct SND_Seq_t
    {
        static constexpr size_t kNameLength = 0x40;
        static constexpr size_t kEntities = 0xC;

        char name[kNameLength];
        i32 header[kEntities];
    };

    struct SND_Set_t
    {
        static constexpr size_t kNameLength = 0x50;

        char name[kNameLength];
    };

    struct SND_Sound_t
    {
        static constexpr size_t kNameLength = 0x50;

        char name[kNameLength];
    };

    struct SND_Wave_t
    {
        static constexpr size_t kMaxIdLength = 0x30;
        static constexpr size_t kMaxNameLength = 0x30;
        static constexpr size_t kDataEntities = 0x14;

        char id[kMaxIdLength];
        char name[kMaxNameLength];
        i32 data[kDataEntities];
    };

    struct SND_t
    {
        // Consts
        static constexpr size_t kHeaderRowsCount = 0x4;
        static constexpr size_t kMaxSoundFilePath = 0x20;

        // Data
        i32 header[kHeaderRowsCount];
        char soundFilePath[kMaxSoundFilePath];
        i32 unknown[4];

        // Rows
    };
}