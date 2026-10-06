#include <Glacier/Filesystem/CFastLookupFileCache_t.h>
#include <Glacier/Filesystem/IOFSAsyncAccess_t.h>
#include <Glacier/Filesystem/ZBufferReader.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/Filesystem/FsZip_t.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>

#include <zlib.h>


// A special check for zlib to check compatibility with PC build
RE_VERIFY_SIZE(z_stream, 0x38); // Same size to PC

namespace Glacier
{
    namespace
    {
        constexpr uint32_t kZipCentralDirSignature = 0x02014B50;
        constexpr uint32_t kZipEndOfCentralDirSignature = 0x06054B50;
        constexpr uint32_t kZipRuneSignature = 0x656E7552;

        bool StartsWithPackPrefix(const char* filename)
        {
            constexpr char prefix[] = "pack.";

            for (size_t i = 0; i < sizeof(prefix) - 1; ++i)
            {
                char lhs = filename[i];
                if (lhs >= 'A' && lhs <= 'Z')
                {
                    lhs = static_cast<char>(lhs - 'A' + 'a');
                }

                if (lhs != prefix[i])
                {
                    return false;
                }
            }

            return true;
        }

        bool WildcardCompareI(const char* str, const char* wildcard)
        {
            if (!*wildcard)
            {
                return false;
            }

            const char* s = str;
            const char* w = wildcard;
            while (true)
            {
                if (*w == '*')
                {
                    const char* nextChunk = w + 1;
                    const char* nextStar = strchr(nextChunk, '*');
                    w = nextStar;

                    if (!nextStar)
                    {
                        const auto strLen = strlen(s);
                        const auto chunkLen = strlen(nextChunk);
                        return strLen >= chunkLen && stricmp(&s[strLen - chunkLen], nextChunk) == 0;
                    }

                    const auto chunkLen = static_cast<size_t>(nextStar - nextChunk);
                    if (!chunkLen)
                    {
                        return false;
                    }

                    const auto strLen = strlen(s);
                    if (strLen < chunkLen)
                    {
                        return false;
                    }

                    const auto maxSearchPos = strLen - chunkLen;
                    size_t checked = 0;
                    while (memicmp(nextChunk, s, chunkLen) != 0)
                    {
                        ++checked;
                        ++s;
                        if (checked == maxSearchPos + 1)
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    const char* nextStar = strchr(w, '*');
                    if (!nextStar)
                    {
                        return stricmp(w, s) == 0;
                    }

                    const auto chunkLen = static_cast<size_t>(nextStar - w);
                    if (memicmp(w, s, chunkLen) != 0)
                    {
                        return false;
                    }

                    s += chunkLen;
                    w = nextStar;
                }

                if (!*w)
                {
                    return true;
                }
            }
        }

        struct FsZipStreamHandle_t : IOFSHandle_t
        {
            IOZip_LocalFileHeader_t header;
            RE_ADD_PADDING(2);
            uint32_t startOffset;
            uint32_t currentOffset;
            z_stream* stream;
            uint32_t compressedSize;
            uint8_t inputBuffer[0x400];
            int inflateResult;
        };

        RE_VERIFY_SIZE(FsZipStreamHandle_t, 0x434);
        RE_VERIFY_OFFSET(FsZipStreamHandle_t, header, 0x4);
        RE_VERIFY_OFFSET(FsZipStreamHandle_t, startOffset, 0x20);
        RE_VERIFY_OFFSET(FsZipStreamHandle_t, currentOffset, 0x24);
        RE_VERIFY_OFFSET(FsZipStreamHandle_t, stream, 0x28);
        RE_VERIFY_OFFSET(FsZipStreamHandle_t, compressedSize, 0x2C);
        RE_VERIFY_OFFSET(FsZipStreamHandle_t, inputBuffer, 0x30);
        RE_VERIFY_OFFSET(FsZipStreamHandle_t, inflateResult, 0x430);

    }

    FsZip_t::~FsZip_t()
    {
        unloadFS();
    }

    FsZip_t::FsZip_t()
    {
        m_pFileCache = nullptr;
        m_bErrorDuringWrite = false;
        fsName = nullptr;
        statusOK = false;
        fsAccess = IOFS_NOACCESS;
        m_icurCompressLevel = COMPRESSLEVEL_DEFAULT;
        m_binvisible = false;
        m_InvalidOffsets.init(256);
        m_pAccess = &DefaultIOFSPhysicalAccess;
    }

    FsZip_t::FsZip_t(void* pBuffer, uint32_t cbBufferSize)
    {
        fsName = static_cast<char*>(ZUniMemory::Allocate(1));
        fsName[0] = '\0';
        m_icurCompressLevel = COMPRESSLEVEL_DEFAULT;

        auto* pMemoryAccess = ZUniMemory::New<IOFSMemoryAccess_t>();
        m_pAccess = pMemoryAccess;
        fp = pMemoryAccess->useBuffer(pBuffer, cbBufferSize);

        statusOK = true;
        fsAccess = IOFS_READ;

        m_cdirArray.init(256);
        m_InvisibleCdirArray.init(8);
        m_InvalidOffsets.init(256);

        memset(&m_Eocd, 0, sizeof(m_Eocd));
        memset(&m_InvisibleEocd, 0, sizeof(m_InvisibleEocd));
        findEOCDOffset();

        m_pFileCache = ZUniMemory::New<CFastLookupFileCache_t>();
        initializeFileCache();
        m_bErrorDuringWrite = false;
    }

    uint8_t* FsZip_t::extractData(IOZip_LocalFileHeader_t& header, void* outbuf, int offset, int outsize)
    {
        auto* inbuf = static_cast<uint8_t*>(ZUniMemory::Allocate(0x20000));
        auto* output = static_cast<uint8_t*>(outbuf);

        if (header.zipBody.compressionMethod == 0)
        {
            m_pAccess->nativeReadTo(output + offset, outsize, fp);
            ZUniMemory::Free(inbuf);
            return output;
        }

        if (header.zipBody.compressionMethod != 8)
        {
            ZUniMemory::Free(inbuf);
            return nullptr;
        }

        z_stream stream {};
        stream.next_out = output + offset;
        stream.avail_out = outsize;

        auto remaining = header.zipBody.compressedSize;
        auto result = inflateInit2(&stream, -15);
        if (remaining)
        {
            while (result == Z_OK)
            {
                uint32_t bytesRead = 0;

                if (m_pAccess->CanReadOverlayed())
                {
                    void* input = nullptr;
                    bytesRead = m_pAccess->nativeReadOL(&input, remaining < 0x20000 ? remaining : 0x20000, fp);
                    stream.next_in = static_cast<Bytef*>(input);
                }
                else
                {
                    bytesRead = m_pAccess->nativeReadTo(inbuf, remaining < 0x20000 ? remaining : 0x20000, fp);
                    stream.next_in = inbuf;
                }

                stream.avail_in = bytesRead;
                if (bytesRead)
                {
                    result = inflate(&stream, bytesRead >= remaining ? Z_FINISH : Z_NO_FLUSH);
                    remaining -= bytesRead;
                }

                if (!remaining)
                {
                    break;
                }
            }
        }

        inflateEnd(&stream);
        ZUniMemory::Free(inbuf);

        return result == Z_STREAM_END || result == Z_OK ? output : nullptr;
    }

    uint32_t FsZip_t::findEOCDOffset()
    {
        m_pAccess->nativeSeek(fp, 0, SEEK_END);
        auto offset = m_pAccess->nativeTell(fp) - 22;
        m_pAccess->nativeSeek(fp, offset, SEEK_SET);

        while (offset >= 0)
        {
            uint32_t signature = 0;
            if (!m_pAccess->nativeReadTo(&signature, sizeof(signature), fp))
            {
                // g_bDisplayBadDisc = true;
                statusOK = false;
                break;
            }

            if (signature == kZipRuneSignature)
            {
                m_pAccess->nativeReadTo(&m_InvisibleEocd, sizeof(m_InvisibleEocd), fp);
                m_pAccess->nativeSeek(fp, -static_cast<int>(sizeof(m_InvisibleEocd)), SEEK_CUR);
            }
            else if (signature == kZipEndOfCentralDirSignature)
            {
                m_iEocdOffset = m_pAccess->nativeTell(fp);
                m_pAccess->nativeReadTo(&m_Eocd, sizeof(m_Eocd), fp);
                return m_iEocdOffset;
            }

            --offset;
            m_pAccess->nativeSeek(fp, -5, SEEK_CUR);
        }

        return 0;
    }

    bool FsZip_t::findFile(const char* psFilename, IOZip_LocalFileHeader_t& header, uint32_t* pOffset)
    {
        bool found = false;
        auto* normalizedFilename = static_cast<char*>(ZUniMemory::Allocate(0x10001));
        auto* filename = static_cast<char*>(ZUniMemory::Allocate(0x10001));
        auto* skipBuffer = static_cast<char*>(ZUniMemory::Allocate(0x10001));

        uint32_t normalizedLength = 0;
        while (psFilename[normalizedLength])
        {
            normalizedFilename[normalizedLength] = psFilename[normalizedLength] == '\\' ? '/' : psFilename[normalizedLength];
            ++normalizedLength;
        }
        normalizedFilename[normalizedLength] = '\0';

        const auto hasWildcard = strchr(psFilename, '*') != nullptr;
        if (!hasWildcard && m_pFileCache && !pOffset)
        {
            int filePos = -1;
            if (m_pFileCache->lookup(normalizedFilename, &header, &filePos))
            {
                m_pAccess->nativeSeek(fp, filePos, SEEK_SET);
                found = true;
            }

            ZUniMemory::Free(normalizedFilename);
            ZUniMemory::Free(filename);
            ZUniMemory::Free(skipBuffer);
            return found;
        }

        const auto cdirOffset = m_InvisibleEocd.cdirSize ? m_InvisibleEocd.cdirOffsetOnStartDisk : m_Eocd.cdirOffsetOnStartDisk;
        m_pAccess->nativeSeek(fp, cdirOffset, SEEK_SET);

        while (!m_pAccess->nativeEof(fp))
        {
            uint32_t signature = 0;
            m_pAccess->nativeReadTo(&signature, sizeof(signature), fp);
            if (signature != kZipCentralDirSignature)
            {
                if (signature != kZipEndOfCentralDirSignature && signature != kZipRuneSignature)
                {
                    // g_bDisplayBadDisc = true;
                    statusOK = false;
                    ZASSERT(false);
                }

                break;
            }

            const auto cdirOffsetInFile = m_pAccess->nativeTell(fp);
            IOZip_CentralDirStructure_t cdir {};
            m_pAccess->nativeReadTo(&cdir, sizeof(cdir), fp);

            if (cdir.zipBody.filenameLength)
            {
                m_pAccess->nativeReadTo(filename, cdir.zipBody.filenameLength, fp);
                filename[cdir.zipBody.filenameLength] = '\0';

                bool invalidated = false;
                for (uint32_t i = 0; i < m_InvalidOffsets.cur; ++i)
                {
                    if (cdirOffsetInFile == m_InvalidOffsets.array[i])
                    {
                        invalidated = true;
                        break;
                    }
                }

                if (!invalidated)
                {
                    bool matches = false;
                    if (hasWildcard)
                    {
                        matches = WildcardCompareI(filename, normalizedFilename);
                    }
                    else if (normalizedLength == cdir.zipBody.filenameLength)
                    {
                        matches = stricmp(filename, normalizedFilename) == 0;
                    }

                    if (matches)
                    {
                        memcpy(&header.zipBody, &cdir.zipBody, sizeof(header.zipBody));
                        found = true;
                    }
                }
            }

            if (cdir.zipBody.extraFieldLength)
            {
                m_pAccess->nativeReadTo(skipBuffer, cdir.zipBody.extraFieldLength, fp);
            }

            if (cdir.fileCommentLength)
            {
                m_pAccess->nativeReadTo(skipBuffer, cdir.fileCommentLength, fp);
            }

            if (found)
            {
                if (pOffset)
                {
                    *pOffset = cdirOffsetInFile;
                }

                const auto filePos = cdir.zipBody.filenameLength + cdir.localHeaderOffset + 30;
                m_pAccess->nativeSeek(fp, filePos, SEEK_SET);

                if (m_pFileCache)
                {
                    m_pFileCache->add(normalizedFilename, &header, m_pAccess->nativeTell(fp));
                }

                break;
            }
        }

        ZUniMemory::Free(normalizedFilename);
        ZUniMemory::Free(filename);
        ZUniMemory::Free(skipBuffer);

        return found;
    }

    void FsZip_t::initializeFileCache()
    {
        auto* filename = static_cast<char*>(ZUniMemory::Allocate(0x10001));
        auto* skipBuffer = static_cast<char*>(ZUniMemory::Allocate(0x10001));

        const auto cdirOffset = m_InvisibleEocd.cdirSize ? m_InvisibleEocd.cdirOffsetOnStartDisk : m_Eocd.cdirOffsetOnStartDisk;
        m_pAccess->nativeSeek(fp, cdirOffset, SEEK_SET);

        if (!m_pAccess->nativeEof(fp))
        {
            while (true)
            {
                bool shouldCache = false;
                uint32_t signature = 0;

                m_pAccess->nativeReadTo(&signature, sizeof(signature), fp);
                if (signature != kZipCentralDirSignature)
                {
                    if (signature != kZipEndOfCentralDirSignature && signature != kZipRuneSignature)
                    {
                        statusOK = false;
                    }

                    break;
                }

                const auto cdirOffsetInFile = m_pAccess->nativeTell(fp);

                IOZip_CentralDirStructure_t cdir {};
                m_pAccess->nativeReadTo(&cdir, sizeof(cdir), fp);

                if (cdir.zipBody.filenameLength)
                {
                    m_pAccess->nativeReadTo(filename, cdir.zipBody.filenameLength, fp);
                    filename[cdir.zipBody.filenameLength] = '\0';

                    bool invalidated = false;
                    for (uint32_t i = 0; i < m_InvalidOffsets.cur; ++i)
                    {
                        if (cdirOffsetInFile == m_InvalidOffsets.array[i])
                        {
                            invalidated = true;
                            break;
                        }
                    }

                    if (!invalidated)
                    {
                        shouldCache = true;
                    }
                }

                if (cdir.zipBody.extraFieldLength)
                {
                    m_pAccess->nativeReadTo(skipBuffer, cdir.zipBody.extraFieldLength, fp);
                }

                if (cdir.fileCommentLength)
                {
                    m_pAccess->nativeReadTo(skipBuffer, cdir.fileCommentLength, fp);
                }

                if (shouldCache)
                {
                    IOZip_LocalFileHeader_t header {};
                    memcpy(&header.zipBody, &cdir.zipBody, sizeof(header.zipBody));
                    m_pFileCache->add(filename, &header, cdir.zipBody.filenameLength + cdir.localHeaderOffset + 30);
                }

                if (m_pAccess->nativeEof(fp))
                {
                    break;
                }
            }
        }

        ZUniMemory::Free(filename);
        ZUniMemory::Free(skipBuffer);
    }

    void FsZip_t::writeCDirs()
    {
        auto writeCDirArray = [this](DynamicArray_t<IOZip_CentralDirStructure_t>& cdirArray, IOZip_EndOfCentralDir_t& eocd)
        {
            eocd.cdirOffsetOnStartDisk = m_pAccess->nativeTell(fp);

            for (uint32_t i = 0; i < cdirArray.cur; ++i)
            {
                auto& cdir = cdirArray.array[i];
                uint32_t signature = kZipCentralDirSignature;

                if (m_pAccess->nativeWrite(&signature, sizeof(signature), fp) != sizeof(signature))
                {
                    m_bErrorDuringWrite = true;
                }

                if (m_pAccess->nativeWrite(&cdir, sizeof(cdir), fp) != sizeof(cdir))
                {
                    m_bErrorDuringWrite = true;
                }

                const auto currentOffset = m_pAccess->nativeTell(fp);
                const auto filenameLength = cdir.zipBody.filenameLength;

                m_pAccess->nativeSeek(fp, cdir.localHeaderOffset + 30, SEEK_SET);

                auto* filename = static_cast<char*>(ZUniMemory::Allocate(filenameLength));
                if (m_pAccess->nativeReadTo(filename, filenameLength, fp) != filenameLength)
                {
                    m_bErrorDuringWrite = true;
                }

                m_pAccess->nativeSeek(fp, currentOffset, SEEK_SET);

                if (m_pAccess->nativeWrite(filename, filenameLength, fp) != filenameLength)
                {
                    m_bErrorDuringWrite = true;
                }

                ZUniMemory::Free(filename);
                ++eocd.numEntriesInCDir;
            }

            eocd.numEntriesInCDirThisDisk = eocd.numEntriesInCDir;
            eocd.cdirSize = m_pAccess->nativeTell(fp) - eocd.cdirOffsetOnStartDisk;
        };

        writeCDirArray(m_InvisibleCdirArray, m_InvisibleEocd);
        writeCDirArray(m_cdirArray, m_Eocd);
    }

    void FsZip_t::writeEOCD()
    {
        uint32_t signature = kZipEndOfCentralDirSignature;
        if (m_pAccess->nativeWrite(&signature, sizeof(signature), fp) != sizeof(signature))
        {
            m_bErrorDuringWrite = true;
        }

        if (m_pAccess->nativeWrite(&m_Eocd, sizeof(m_Eocd), fp) != sizeof(m_Eocd))
        {
            m_bErrorDuringWrite = true;
        }

        signature = kZipRuneSignature;
        if (m_pAccess->nativeWrite(&signature, sizeof(signature), fp) != sizeof(signature))
        {
            m_bErrorDuringWrite = true;
        }

        if (m_pAccess->nativeWrite(&m_InvisibleEocd, sizeof(m_InvisibleEocd), fp) != sizeof(m_InvisibleEocd))
        {
            m_bErrorDuringWrite = true;
        }
    }

    void FsZip_t::writeLocalFile(const char* psFilename, _FILETIME* pFileTime, IBuffer* pBuffer)
    {
        (void)psFilename;
        (void)pFileTime;
        (void)pBuffer;

        if (fsAccess == IOFS_CREATE || fsAccess == IOFS_WRITE)
        {
            auto bValid = m_pAccess->nativeFdValid(fp);
            if (!bValid)
            {
                m_bErrorDuringWrite = true;
            }

            ZASSERT(bValid);
        }
        else
        {
            // IOI...
            m_bErrorDuringWrite = true;
        }
    }

    void FsZip_t::PrintStatus()
    {
        if (statusOK)
        {
            printf("fs: '%s' Offset = %i, status = %i\n", fsName, m_pAccess->nativeTell(fp), errorStatus);
        }
        else
        {
            printf("fs not initialized\n");
        }
    }

    void FsZip_t::Add(const char* name, _FILETIME* filetime, IBuffer* pBuffer)
    {
        writeLocalFile(name, filetime, pBuffer);
    }

    void FsZip_t::Add(const char* name, _FILETIME* filetime, void* data, uint32_t datalen)
    {
        ZBufferReader reader {};
        reader.InstallBuffer(data, datalen, 0);

        writeLocalFile(name, filetime, &reader);
    }

    void FsZip_t::Add(const char* Name, char* pszNameInFile)
    {
        if (fsAccess == IOFS_CREATE || fsAccess == IOFS_WRITE)
        {
            char mode[] = "rb";
            auto* pFile = m_pAccess->nativeOpen(Name, mode);

            if (pFile)
            {
                writeLocalFile(pszNameInFile, nullptr, reinterpret_cast<IBuffer*>(pFile));
                m_pAccess->nativeClose(pFile);
            }
        }
        else
        {
            m_bErrorDuringWrite = true;
        }
    }
    
    void FsZip_t::Save(const char* filename)
    {
        printf("ZIPFS: Save: This method is unsupported by this filesystem\n");
    }

    int FsZip_t::GetSize(const char* filename)
    {
        IOZip_LocalFileHeader_t header {};
        auto bFound = findFile(filename, header, nullptr);

        if (!bFound)
        {
            return -1;
        }

        return header.zipBody.uncompressedSize;
    }

    bool FsZip_t::Exists(const char* filename)
    {
        IOZip_LocalFileHeader_t header;
        return findFile(filename, header, nullptr) != 0;
    }

    bool FsZip_t::GetFileTime(const char* filename, _FILETIME* pFiletime)
    {
        ZASSERT(false); // Unsupported by IOI
        return false;
    }

    int FsZip_t::Load(const char* filename, void* ptr, int size, int offset)
    {
        IOZip_LocalFileHeader_t fileHeader{};
        if (!findFile(filename, fileHeader, nullptr))
            return -1;
            
        if (!fileHeader.zipBody.uncompressedSize)
        {
            if (extractData(fileHeader, ptr, 0, size))
            {
                return size;
            }
        }

        return -1;
    }
    
    void FsZip_t::GetDirectory(STRREFTAB* prtFiles)
    {
        auto* filename = static_cast<char*>(ZUniMemory::Allocate(0x10001));
        const auto cdirOffset = m_InvisibleEocd.cdirSize ? m_InvisibleEocd.cdirOffsetOnStartDisk : m_Eocd.cdirOffsetOnStartDisk;

        m_pAccess->nativeSeek(fp, cdirOffset, SEEK_SET);

        while (!m_pAccess->nativeEof(fp))
        {
            uint32_t signature = 0;
            const auto bytesRead = m_pAccess->nativeReadTo(&signature, sizeof(signature), fp);

            if (signature == kZipCentralDirSignature)
            {
                const auto cdirOffsetInFile = m_pAccess->nativeTell(fp);

                IOZip_CentralDirStructure_t cdir {};
                m_pAccess->nativeReadTo(&cdir, sizeof(cdir), fp);

                if (cdir.zipBody.filenameLength)
                {
                    m_pAccess->nativeReadTo(filename, cdir.zipBody.filenameLength, fp);
                    filename[cdir.zipBody.filenameLength] = '\0';

                    bool invalidated = false;
                    for (uint32_t i = 0; i < m_InvalidOffsets.cur; ++i)
                    {
                        if (cdirOffsetInFile == m_InvalidOffsets.array[i])
                        {
                            invalidated = true;
                            break;
                        }
                    }

                    if (!invalidated)
                    {
                        prtFiles->AddStr(filename);
                    }
                }

                if (cdir.zipBody.extraFieldLength)
                {
                    m_pAccess->nativeSeek(fp, cdir.zipBody.extraFieldLength, SEEK_CUR);
                }

                if (cdir.fileCommentLength)
                {
                    m_pAccess->nativeSeek(fp, cdir.fileCommentLength, SEEK_CUR);
                }
            }
            else
            {
                if (signature == kZipEndOfCentralDirSignature || signature == kZipRuneSignature)
                {
                    break;
                }

                ZASSERT(bytesRead == sizeof(signature));
                break;
            }
        }

        ZUniMemory::Free(filename);
    }
    
    void FsZip_t::SyncClose()
    {
        m_cdirArray.cleanup();
        m_InvisibleCdirArray.cleanup();
        m_InvalidOffsets.cleanup();

        m_pAccess->nativeClose(fp);
        statusOK = false;

        if (m_pFileCache)
        {
            ZUniMemory::Delete(m_pFileCache);
            m_pFileCache = nullptr;
        }
    }
    
    void FsZip_t::SyncOpen()
    {
        // Copy our name to temp buffer, release old ptr and ask initFS to init to copy acquire that pointer back
        char aBuffer[264] { 0 };
        strncpy(aBuffer, fsName, 260);

        auto* psOldFsName = fsName;
        aBuffer[260] = 0;

        ZUniMemory::Free(psOldFsName);
        fsName = 0;

        initFS(aBuffer, fsAccess);
    }
    
    void FsZip_t::invalidateRedundantFiles(FsZip_t& zipfs)
    {
        bool done = false;
        auto* filename = static_cast<char*>(ZUniMemory::Allocate(0x10001));
        auto* skipBuffer = static_cast<char*>(ZUniMemory::Allocate(0x10001));

        m_pAccess->nativeSeek(zipfs.fp, zipfs.m_Eocd.cdirOffsetOnStartDisk, SEEK_SET);

        while (!m_pAccess->nativeEof(zipfs.fp))
        {
            if (done)
            {
                break;
            }

            uint32_t signature = 0;
            m_pAccess->nativeReadTo(&signature, sizeof(signature), zipfs.fp);
            const auto cdirOffsetInFile = m_pAccess->nativeTell(zipfs.fp);

            if (signature == kZipCentralDirSignature)
            {
                IOZip_CentralDirStructure_t cdir {};
                m_pAccess->nativeReadTo(&cdir, sizeof(cdir), zipfs.fp);

                if (cdir.zipBody.filenameLength)
                {
                    m_pAccess->nativeReadTo(filename, cdir.zipBody.filenameLength, zipfs.fp);
                    filename[cdir.zipBody.filenameLength] = '\0';

                    const auto filenameEndOffset = m_pAccess->nativeTell(zipfs.fp);
                    IOZip_LocalFileHeader_t header {};
                    if (findFile(filename, header, nullptr))
                    {
                        FsZip_t* zipfsToInvalidate = nullptr;

                        if (header.zipBody.olderThan(cdir.zipBody))
                        {
                            if (!StartsWithPackPrefix(filename))
                            {
                                g_pSysFile->IsLoadedByLoadfilter(filename);
                            }

                            zipfsToInvalidate = this;
                        }
                        else
                        {
                            if (cdir.zipBody.olderThan(header.zipBody) && !StartsWithPackPrefix(filename))
                            {
                                g_pSysFile->IsLoadedByLoadfilter(filename);
                            }

                            zipfsToInvalidate = &zipfs;
                        }

                        zipfsToInvalidate->InvalidateFile(filename);
                    }

                    m_pAccess->nativeSeek(zipfs.fp, filenameEndOffset, SEEK_SET);
                }

                if (cdir.zipBody.extraFieldLength)
                {
                    m_pAccess->nativeReadTo(skipBuffer, cdir.zipBody.extraFieldLength, zipfs.fp);
                }

                if (cdir.fileCommentLength)
                {
                    m_pAccess->nativeReadTo(skipBuffer, cdir.fileCommentLength, zipfs.fp);
                }
            }
            else if (signature == kZipEndOfCentralDirSignature || signature == kZipRuneSignature)
            {
                done = true;
            }
            else
            {
                (void)cdirOffsetInFile;
                ZASSERT(false);
            }
        }

        ZUniMemory::Free(filename);
        ZUniMemory::Free(skipBuffer);

    }

    bool FsZip_t::initFS(const char* fsname, IOFSAccess_t access)
    {
        if (fsName)
        {
            unloadFS();
        }

        switch (access)
        {
        case IOFS_PRECACHE:
            m_pAccess = ZUniMemory::New<IOFSMemoryAccess_t>();
            break;
        case IOFS_ASYNC:
            m_pAccess = ZUniMemory::New<IOFSAsyncAccess_t>();
            break;
        default:
            break;
        }

        const char* mode = access == IOFS_CREATE ? "wb+" : "rb";
        fp = m_pAccess->nativeOpen(fsname, const_cast<char*>(mode));
        if (!fp)
        {
            fsAccess = IOFS_NOACCESS;
            return false;
        }

        const auto fsNameLength = strlen(fsname) + 1;
        fsName = static_cast<char*>(ZUniMemory::Allocate(fsNameLength));
        memcpy(fsName, fsname, fsNameLength);
        fsAccess = access;
        statusOK = true;

        m_cdirArray.init(256);
        m_InvisibleCdirArray.init(8);
        m_InvalidOffsets.init(256);

        memset(&m_Eocd, 0, sizeof(m_Eocd));
        memset(&m_InvisibleEocd, 0, sizeof(m_InvisibleEocd));
        findEOCDOffset();

        m_pFileCache = ZUniMemory::New<CFastLookupFileCache_t>();
        if (access != IOFS_CREATE)
        {
            initializeFileCache();
        }

        m_bErrorDuringWrite = false;
        return true;
    }

    void FsZip_t::unloadFS()
    {
        if (!fsName)
        {
            return;
        }

        if (fsAccess == IOFS_CREATE || fsAccess == IOFS_WRITE)
        {
            writeCDirs();
            writeEOCD();
        }

        m_cdirArray.cleanup();
        m_InvisibleCdirArray.cleanup();

        ZUniMemory::Free(fsName);
        fsName = nullptr;

        m_pAccess->nativeClose(fp);
        statusOK = false;
        fsAccess = IOFS_NOACCESS;

        if (m_pFileCache)
        {
            ZUniMemory::Delete(m_pFileCache);
            m_pFileCache = nullptr;
        }

        if (m_pAccess != &DefaultIOFSPhysicalAccess)
        {
            ZUniMemory::Delete(m_pAccess);
            m_pAccess = &DefaultIOFSPhysicalAccess;
        }
    }

    IOFSHandle_t* FsZip_t::open(const char* filename, IOFSAccess_t access)
    {
        auto* handle = static_cast<FsZipStreamHandle_t*>(ZUniMemory::Allocate(sizeof(FsZipStreamHandle_t)));
        if (!handle)
        {
            return nullptr;
        }

        memset(handle, 0, sizeof(FsZipStreamHandle_t));
        handle->stream = static_cast<z_stream*>(ZUniMemory::Allocate(sizeof(z_stream)));
        if (!handle->stream)
        {
            ZUniMemory::Free(handle);
            return nullptr;
        }

        memset(handle->stream, 0, sizeof(z_stream));
        handle->fileAccess = access;
        handle->startOffset = 0;

        if (access == IOFS_READ && findFile(filename, handle->header, nullptr))
        {
            const auto currentOffset = m_pAccess->nativeTell(fp);
            handle->startOffset = currentOffset;
            handle->currentOffset = currentOffset;
            handle->compressedSize = handle->header.zipBody.compressedSize;
            handle->stream->zalloc = nullptr;
            handle->stream->zfree = nullptr;
            handle->stream->next_in = handle->inputBuffer;
            handle->stream->avail_in = 0;

            handle->inflateResult = inflateInit2(handle->stream, -15);
            if (handle->inflateResult == Z_OK)
            {
                m_bErrorDuringWrite = false;
                return handle;
            }
        }

        ZUniMemory::Free(handle->stream);
        ZUniMemory::Free(handle);
        return nullptr;
    }
    
    int FsZip_t::read(IOFSHandle_t* fh, void* buffer, unsigned int len)
    {
        auto* handle = static_cast<FsZipStreamHandle_t*>(fh);

        m_pAccess->nativeSeek(fp, handle->startOffset, SEEK_SET);

        if (handle->header.zipBody.compressionMethod)
        {
            if (handle->header.zipBody.compressionMethod == 8 && handle->inflateResult == Z_OK)
            {
                handle->stream->next_out = static_cast<Bytef*>(buffer);
                handle->stream->avail_out = len;

                while (handle->stream->avail_out)
                {
                    if (handle->inflateResult)
                    {
                        break;
                    }

                    const auto compressedBytesRead = handle->startOffset - handle->currentOffset;
                    auto bytesToRead = sizeof(handle->inputBuffer) - handle->stream->avail_in;
                    if (handle->header.zipBody.compressedSize - compressedBytesRead < bytesToRead)
                    {
                        bytesToRead = handle->header.zipBody.compressedSize - compressedBytesRead;
                    }

                    if (bytesToRead)
                    {
                        handle->stream->avail_in += m_pAccess->nativeReadTo(
                            handle->inputBuffer + handle->stream->avail_in,
                            bytesToRead,
                            fp);
                    }

                    handle->stream->next_in = handle->inputBuffer;
                    handle->inflateResult = inflate(handle->stream, Z_SYNC_FLUSH);
                    memmove(handle->inputBuffer, handle->stream->next_in, handle->stream->avail_in);
                }

                handle->startOffset = m_pAccess->nativeTell(fp);
                return len - handle->stream->avail_out;
            }

            return 0;
        }

        m_pAccess->nativeSeek(fp, handle->startOffset, SEEK_SET);
        const auto bytesRead = m_pAccess->nativeReadTo(buffer, len, fp);
        handle->startOffset = m_pAccess->nativeTell(fp);

        return bytesRead;
    }
    
    int FsZip_t::write(IOFSHandle_t* fh, void* buffer, unsigned int len)
    {
        printf("ZIPFS: Stream write not implemented yet\n");
        return -1;
    }
    
    void FsZip_t::close(IOFSHandle_t* fh)
    {
        auto* handle = static_cast<FsZipStreamHandle_t*>(fh);

        inflateEnd(handle->stream);
        ZUniMemory::Free(handle->stream);
        ZUniMemory::Free(handle);
    }

    bool FsZip_t::eof(IOFSHandle_t* fh)
    {
        auto* handle = static_cast<FsZipStreamHandle_t*>(fh);
        return handle->inflateResult == Z_STREAM_END;
    }

    bool FsZip_t::Compare(const char* filename1, const char* filename2)
    {
        IOZip_LocalFileHeader_t h1, h2;

        return  findFile(filename1, h1, nullptr) && findFile(filename2, h2, nullptr) && 
                h1.zipBody.compressedSize == h2.zipBody.compressedSize && 
                h1.zipBody.crc32 == h2.zipBody.crc32;
    }

    bool FsZip_t::InvalidateFile(const char* filename)
    {
        IOZip_LocalFileHeader_t header {};
        uint32_t offset = 0;
        if (!findFile(filename, header, &offset))
        {
            return false;
        }

        *m_InvalidOffsets.getNextElm() = offset;

        if (m_pFileCache)
        {
            auto* normalizedFilename = static_cast<char*>(ZUniMemory::Allocate(0x10001));
            uint32_t i = 0;

            while (filename[i])
            {
                normalizedFilename[i] = filename[i] == '\\' ? '/' : filename[i];
                ++i;
            }

            normalizedFilename[i] = '\0';
            m_pFileCache->invalidate(normalizedFilename);
            ZUniMemory::Free(normalizedFilename);
        }

        return true;
    }

    void FsZip_t::setCompressionLevel(CompressLevel_t eLevel)
    {
        m_icurCompressLevel = eLevel;
    }

    void FsZip_t::usePrimaryArchive()
    {
        m_binvisible = false;
    }

    void FsZip_t::useSecondaryArchive()
    {
        m_binvisible = true;
    }
    
    bool FsZip_t::AddZipFSArchive(const char* psArchive)
    {
        // Not implemented by IOI
        ZASSERT(false);
        return false;
    }

    bool FsZip_t::hadErrorsDuringWrite() const
    {
        return m_bErrorDuringWrite;
    }
}
