#pragma once

//
// All IOI ZIP stuff stored here
// 

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    enum CompressLevel_t : uint32_t
    {
        COMPRESSLEVEL_NONE = 0x0u,
        COMPRESSLEVEL_MAXSPEED = 0x1u,
        COMPRESSLEVEL_2 = 0x2u,
        COMPRESSLEVEL_3 = 0x3u,
        COMPRESSLEVEL_4 = 0x4u,
        COMPRESSLEVEL_5 = 0x5u,
        COMPRESSLEVEL_6 = 0x6u,
        COMPRESSLEVEL_7 = 0x7u,
        COMPRESSLEVEL_8 = 0x8u,
        COMPRESSLEVEL_MINSIZE = 0x9,
        COMPRESSLEVEL_DEFAULT = 0xFFFFFFFFu,
    };

#pragma pack(push, 2)  /// ??? Need confirm for PC
    struct IOZip_Body_t
    {
        // methods
        bool olderThan(const IOZip_Body_t& sBody) const
        {
            const auto otherYear = sBody.lastModFileDate >> 9;
            const auto thisYear = lastModFileDate >> 9;
            if (thisYear < otherYear) return true;
            if (thisYear > otherYear) return false;

            const auto otherMonth = (sBody.lastModFileDate >> 5) & 0xF;
            const auto thisMonth = (lastModFileDate >> 5) & 0xF;
            if (thisMonth < otherMonth) return true;
            if (thisMonth > otherMonth) return false;

            const auto otherDay = sBody.lastModFileDate & 0x1F;
            const auto thisDay = lastModFileDate & 0x1F;
            if (thisDay < otherDay) return true;
            if (thisDay > otherDay) return false;

            const auto otherHour = sBody.lastModFileTime >> 11;
            const auto thisHour = lastModFileTime >> 11;
            if (thisHour < otherHour) return true;
            if (thisHour > otherHour) return false;

            const auto otherMinute = (sBody.lastModFileTime >> 5) & 0x3F;
            const auto thisMinute = (lastModFileTime >> 5) & 0x3F;
            if (thisMinute < otherMinute) return true;
            if (thisMinute > otherMinute) return false;

            return (lastModFileTime & 0x1F) < (sBody.lastModFileTime & 0x1F);
        }

        // members
        uint16_t versionExtract{};
        uint16_t generalPurpose{};
        uint16_t compressionMethod{};
        uint16_t lastModFileTime{};
        uint16_t lastModFileDate{};
        uint32_t crc32{};
        uint32_t compressedSize{};
        uint32_t uncompressedSize{};
        uint16_t filenameLength{};
        uint16_t extraFieldLength{};
    };
    RE_VERIFY_SIZE(IOZip_Body_t, 0x1A);

    struct IOZip_CentralDirStructure_t
    {
        // members
        uint16_t versionMadeBy{};
        IOZip_Body_t zipBody{};
        uint16_t fileCommentLength{};
        uint16_t diskNumberStart{};
        uint16_t internalFileAttrib{};
        uint32_t externalFileAttrib{};
        uint32_t localHeaderOffset{};
    };
    RE_VERIFY_SIZE(IOZip_CentralDirStructure_t, 0x2A);
    
    struct IOZip_EndOfCentralDir_t
    {
        // methods
        IOZip_EndOfCentralDir_t();
        void reset();

        // members
        uint16_t diskNumber;
        uint16_t diskNumberWithStartOfCDir;
        uint16_t numEntriesInCDir;
        uint16_t numEntriesInCDirThisDisk;
        int cdirSize;
        int cdirOffsetOnStartDisk;
        uint16_t commentLength;
    };
    RE_VERIFY_SIZE(IOZip_EndOfCentralDir_t, 0x12);
    
    // Constraints
    RE_VERIFY_OFFSET(IOZip_EndOfCentralDir_t, diskNumber, 0x0);
    RE_VERIFY_OFFSET(IOZip_EndOfCentralDir_t, diskNumberWithStartOfCDir, 0x2);
    RE_VERIFY_OFFSET(IOZip_EndOfCentralDir_t, numEntriesInCDir, 0x4);
    RE_VERIFY_OFFSET(IOZip_EndOfCentralDir_t, numEntriesInCDirThisDisk, 0x6);
    RE_VERIFY_OFFSET(IOZip_EndOfCentralDir_t, cdirSize, 0x8);
    RE_VERIFY_OFFSET(IOZip_EndOfCentralDir_t, cdirOffsetOnStartDisk, 0xC);
    RE_VERIFY_OFFSET(IOZip_EndOfCentralDir_t, commentLength, 0x10);
#pragma pack(pop)

    struct IOZip_LocalFileHeader_t
    {
        // members
        IOZip_Body_t zipBody{};
    };
    RE_VERIFY_SIZE(IOZip_LocalFileHeader_t, 0x1A);

    struct IOZip_DataDescriptor_t
    {
        // methods
        IOZip_DataDescriptor_t() = default;

        // members
        uint32_t crc32{0};
        uint32_t compressedSize{0};
        uint32_t uncompressedSize{0};
    };
    RE_VERIFY_SIZE(IOZip_DataDescriptor_t, 0xC);
}
