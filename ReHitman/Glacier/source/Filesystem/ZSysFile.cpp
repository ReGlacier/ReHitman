#include <Glacier/Filesystem/IOFilesystem_t.h>
#include <Glacier/ZSysInterface.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/Filesystem/ZBigFile.h>
#include <Glacier/Filesystem/FsZip_t.h>
#include <Glacier/Filesystem/IBuffer.h>
#include <Glacier/ZSTL/CHUNK.h>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    namespace
    {
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

    }

    // ZSysFile
    bool ZSysFile::IsLoadedByLoadfilter(const char* pFileName)
    {
        if (m_pLoadFilter)
        {
            const auto* extension = strrchr(pFileName, '.');
            if (extension)
            {
                for (const auto* filter : *m_pLoadFilter)
                {
                    const bool matches = strchr(filter, '*') ? WildcardCompareI(pFileName, filter) : stricmp(extension, filter) == 0;
                    if (matches)
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    void ZSysFile::MakeDirNested(const char* DirName)
    {
        auto* dirName = static_cast<char*>(ZUniMemory::Allocate(strlen(DirName) + 1));
        strcpy(dirName, DirName);

        const auto len = strlen(dirName);
        if (len && (dirName[len - 1] == '\\' || dirName[len - 1] == '/'))
        {
            dirName[len - 1] = '\0';
        }

        for (auto* separator = strpbrk(dirName, "/\\"); separator; separator = strpbrk(separator + 1, "/\\"))
        {
            *separator = '\0';
            if (strpbrk(dirName, "/\\"))
            {
                MakeDir(dirName);
            }
            *separator = '/';
        }

        MakeDir(dirName);
        ZUniMemory::Free(dirName);
    }

    void ZSysFile::SetFilenameMangling(bool)
    {
        // Do nothing
    }

    IOFilesystem_t* ZSysFile::GetZipFile(const char* pName)
    {
        if (pName && m_pBigFiles)
        {
            for (auto* filesystem : m_pBigFiles->As<IOFilesystem_t*>())
            {
                if (filesystem->Exists(pName))
                {
                    return filesystem;
                }
            }
        }

        return nullptr;
    }

    void ZSysFile::PrintBigFiles()
    {
        if (m_pBigFiles)
        {
            for (auto* filesystem : m_pBigFiles->As<IOFilesystem_t*>())
            {
                if (filesystem->Exists("Pack.*"))
                {
                    printf("Zip file \"%s\" pack info\n", filesystem->fsName);
                }
                else
                {
                    printf("Zip file \"%s\"\n", filesystem->fsName);
                }
            }
        }
    }

    void ZSysFile::SetLoadFilter(const char* pLoadFilter)
    {
        if (!m_pLoadFilter)
        {
            m_pLoadFilter = ZUniMemory::New<STRREFTAB>(8, 0);
        }

        auto* loadFilter = static_cast<char*>(ZUniMemory::Allocate(strlen(pLoadFilter) + 1));
        strcpy(loadFilter, pLoadFilter);

        auto* current = loadFilter;
        if (*current)
        {
            while (true)
            {
                auto* separator = strchr(current, ';');
                if (!separator)
                {
                    break;
                }

                *separator = '\0';
                m_pLoadFilter->AddAlways(current);
                current = separator + 1;

                if (!*current)
                {
                    ZUniMemory::Free(loadFilter);
                    return;
                }
            }

            m_pLoadFilter->AddAlways(current);
        }

        ZUniMemory::Free(loadFilter);
    }

    void ZSysFile::RemoveLoadFilter(const char* pExtension)
    {
        if (m_pLoadFilter)
        {
            m_pLoadFilter->RemoveStr(pExtension);
        }
    }

    void ZSysFile::CreateListOfBigFileSizes(REFTAB* prtBigFilesSizes)
    {
        if (m_pBigFiles)
        {
            for (auto* filesystem : m_pBigFiles->As<IOFilesystem_t*>())
            {
                auto size = g_pSysFile->GetSize(filesystem->fsName, false);
                if (size < 0)
                {
                    size = 0;
                }

                prtBigFilesSizes->Add(size);
            }
        }
    }

    void ZSysFile::InvalidateZipFile(const char* pFileName)
    {
        if (m_pBigFiles)
        {
            const auto* filename = RemoveSysPath(pFileName);
            if (!filename)
            {
                return;
            }

            for (auto* filesystem : m_pBigFiles->As<IOFilesystem_t*>())
            {
                filesystem->InvalidateFile(filename);
            }
        }
    }

    void ZSysFile::LoadWholeSceneZip(const char* pFileName)
    {
        UseBig(pFileName, true);
    }

    bool ZSysFile::UseBig(CHUNKFILE* pChunkBigFile, const char* pName)
    {
        if (!m_pBigFiles)
        {
            m_pBigFiles = ZUniMemory::New<LINKREFTAB>(4, 1);
        }

        auto* bigFile = ZUniMemory::New<ZBigFile>(pChunkBigFile, pName);
        m_pBigFiles->AddEnd(reinterpret_cast<uint32_t>(bigFile));

        return true;
    }

    bool ZSysFile::UseBig(const char* pFileName, bool bLoadWhole)
    {
        AddBig(pFileName, bLoadWhole);
        return true;
    }

    bool ZSysFile::RemoveBig(const char* pFileName)
    {
        MYSTR filename { pFileName };
        for (int i = 0; i < filename.Length(); ++i)
        {
            if (filename.String[i] == '/')
            {
                filename.String[i] = '\\';
            }
        }

        if (m_pBigFiles)
        {
            RefRun it;
            m_pBigFiles->RunInitNxtRef(&it);

            auto* ref = m_pBigFiles->RunNxtRefPtr(&it);
            while (ref)
            {
                auto* filesystem = reinterpret_cast<IOFilesystem_t*>(*ref);
                if (stricmp(filename.String, filesystem->fsName) == 0)
                {
                    filesystem->unloadFS();
                    ZUniMemory::Delete(filesystem);
                    m_pBigFiles->RunDelRef(&it);

                    if (!m_pBigFiles->Count())
                    {
                        ZUniMemory::Delete(m_pBigFiles);
                        m_pBigFiles = nullptr;
                    }

                    return true;
                }

                ref = m_pBigFiles->RunNxtRefPtr(&it);
            }
        }

        return false;
    }

    void ZSysFile::SyncOpenBig(const char* pName)
    {
        if (m_pBigFiles)
        {
            MYSTR name { pName };
            for (int i = 0; i < name.Length(); ++i)
            {
                if (name.String[i] == '\\')
                {
                    name.String[i] = '/';
                }
            }

            for (auto* filesystem : m_pBigFiles->As<IOFilesystem_t*>())
            {
                if (stricmp(filesystem->fsName, name.String) == 0)
                {
                    filesystem->SyncOpen();
                }
            }
        }
    }

    void ZSysFile::SyncCloseBig(const char* pName)
    {
        if (m_pBigFiles)
        {
            MYSTR name { pName };
            for (int i = 0; i < name.Length(); ++i)
            {
                if (name.String[i] == '\\')
                {
                    name.String[i] = '/';
                }
            }

            for (auto* filesystem : m_pBigFiles->As<IOFilesystem_t*>())
            {
                if (stricmp(filesystem->fsName, name.String) == 0)
                {
                    filesystem->SyncClose();
                }
            }
        }
    }

    bool ZSysFile::BigLoaded(const char* pFileName)
    {
        MYSTR filename { pFileName };
        for (int i = 0; i < filename.Length(); ++i)
        {
            if (filename.String[i] == '\\')
            {
                filename.String[i] = '/';
            }
        }

        if (m_pBigFiles)
        {
            for (auto* filesystem : m_pBigFiles->As<IOFilesystem_t*>())
            {
                if (stricmp(filename.String, filesystem->fsName) == 0)
                {
                    return true;
                }
            }
        }

        return false;
    }

    void ZSysFile::RemoveAllBigs()
    {
        if (m_pBigFiles)
        {
            RefRun it;
            m_pBigFiles->RunInitNxtRef(&it);

            auto* ref = m_pBigFiles->RunNxtRefPtr(&it);
            while (ref)
            {
                auto* filesystem = reinterpret_cast<IOFilesystem_t*>(*ref);
                filesystem->unloadFS();
                ZUniMemory::Delete(filesystem);
                m_pBigFiles->RunDelRef(&it);

                ref = m_pBigFiles->RunNxtRefPtr(&it);
            }

            if (!m_pBigFiles->Count())
            {
                ZUniMemory::Delete(m_pBigFiles);
                m_pBigFiles = nullptr;
            }
        }
    }

    FsZip_t* ZSysFile::CreateZipFile(const char* name)
    {
        auto* zipFile = ZUniMemory::New<FsZip_t>();

        auto* directory = static_cast<char*>(ZUniMemory::Allocate(strlen(name) + 1));
        strcpy(directory, name);

        auto* separator = strrchr(directory, '\\');
        if (!separator)
        {
            separator = strrchr(directory, '/');
        }

        if (separator)
        {
            *separator = '\0';
            MakeDirNested(directory);
        }

        ZUniMemory::Free(directory);
        zipFile->initFS(name, IOFS_CREATE);

        return zipFile;
    }

    void ZSysFile::CloseZipFile(IOFilesystem_t* pZfs)
    {
        pZfs->unloadFS();
        ZUniMemory::Delete(pZfs);
    }

    void ZSysFile::AddToZipFile(IOFilesystem_t* pZipfs, const char* name, const void* pData, uint32_t lSize)
    {
        pZipfs->Add(name, nullptr, const_cast<void*>(pData), lSize);
    }

    bool ZSysFile::CheckOtherZipExt(const char* pName, const char* extList)
    {
        MYSTR name { pName };
        for (int i = 0; i < name.Length(); ++i)
        {
            if (name.String[i] == '/')
            {
                name.String[i] = '\\';
            }
        }

        if (m_pBigFiles)
        {
            for (auto* filesystem : m_pBigFiles->As<IOFilesystem_t*>())
            {
                if (stricmp(filesystem->fsName, name.String) != 0)
                {
                    continue;
                }

                STRREFTAB files { 256, 0 };
                filesystem->GetDirectory(&files);

                for (const auto* filename : files)
                {
                    bool matchesAnyExtension = false;
                    const auto* extension = extList;

                    if (!extension)
                    {
                        return true;
                    }

                    while (*extension)
                    {
                        const auto* separator = strchr(extension, ',');
                        char pattern[1088] {};
                        strcpy(pattern, "*.");

                        if (separator)
                        {
                            const auto length = static_cast<size_t>(separator - extension);
                            strncpy(&pattern[2], extension, length);
                            pattern[length + 2] = '\0';
                            extension = separator + 1;
                        }
                        else
                        {
                            strcpy(&pattern[2], extension);
                            extension += strlen(extension);
                        }

                        if (WildcardCompareI(filename, pattern))
                        {
                            matchesAnyExtension = true;
                            break;
                        }
                    }

                    if (!matchesAnyExtension)
                    {
                        return true;
                    }
                }

                return false;
            }
        }

        return false;
    }

    void ZSysFile::AddExclusiveAllFiles(CHUNK* pChunk, const char* exclusiveList)
    {
        if (!m_pBigFiles)
        {
            return;
        }

        for (auto* filesystem : m_pBigFiles->As<IOFilesystem_t*>())
        {
            STRREFTAB files { 256, 0 };
            filesystem->GetDirectory(&files);

            if (!exclusiveList)
            {
                ZASSERT(false);
                continue;
            }

            for (const auto* filename : files)
            {
                bool excluded = false;
                const auto* extension = exclusiveList;

                while (*extension)
                {
                    const auto* separator = strchr(extension, ',');
                    char pattern[1136] {};
                    strcpy(pattern, "*.");

                    if (separator)
                    {
                        const auto length = static_cast<size_t>(separator - extension);
                        strncpy(&pattern[2], extension, length);
                        pattern[length + 2] = '\0';
                        extension = separator + 1;
                    }
                    else
                    {
                        strcpy(&pattern[2], extension);
                        extension = nullptr;
                    }

                    if (WildcardCompareI(filename, pattern))
                    {
                        excluded = true;
                        break;
                    }

                    if (!extension)
                    {
                        break;
                    }
                }

                if (excluded)
                {
                    continue;
                }

                void* data = nullptr;
                int size = -1;
                bool shouldFreeData = false;

                if (!ExistsInZip(filename))
                {
                    // Original code converts ZIP-style paths to HD paths before trying the physical filesystem.
                    auto convertedFilename = ConvertFileNameFromZipToHD(filename);
                    size = GetSize(convertedFilename.String, true);
                    if (size != -1)
                    {
                        data = ZUniMemory::Allocate(size);
                        if (Load(convertedFilename.String, data, size, 0, true) == size)
                        {
                            shouldFreeData = true;
                        }
                        else
                        {
                            ZUniMemory::Free(data);
                            data = nullptr;
                            size = -1;
                        }
                    }
                }

                if (!data)
                {
                    size = filesystem->GetSize(filename);
                    if (size == -1)
                    {
                        continue;
                    }

                    data = ZUniMemory::Allocate(size);
                    filesystem->Load(filename, data, size, 0);
                    shouldFreeData = true;
                }

                auto* fileChunk = pChunk->AddChunk(1);

                char normalizedName[512] {};
                const auto nameLength = strlen(filename);
                for (size_t i = 0; i <= nameLength && i < sizeof(normalizedName); ++i)
                {
                    normalizedName[i] = filename[i] == '/' ? '\\' : filename[i];
                }

                fileChunk->AddString(normalizedName);
                fileChunk->AddChunk(size)->AddData(data, size);

                if (shouldFreeData)
                {
                    ZUniMemory::Free(data);
                }
            }
        }
    }

    bool ZSysFile::ExistsInZip(const char* pFileName)
    {
        const auto* filename = RemoveSysPath(pFileName);
        return GetZipFile(filename) != nullptr;
    }

    MYSTR ZSysFile::ConvertFilename(const char* pszFilename)
    {
        return MYSTR { pszFilename };
    }

    MYSTR* ZSysFile::ConvertFilename(MYSTR* result, const char* pszFilename)
    {
        result->SetString(pszFilename);
        return result;
    }

    bool ZSysFile::PrepareSaveGame(const char* , const char* , char* , int, bool)
    {
        return false;
    }

    void ZSysFile::SetSaveGamePath(const char* pszSaveGamePath)
    {
        MYSTR saveGamePath { pszSaveGamePath };
        m_sSaveGamePath = saveGamePath;

        // TODO: Finish me after CSharedCom will be reversed.
        // const auto length = m_sSaveGamePath.Length();
        // const auto* value = m_sSaveGamePath.String;
        // g_pGlobalCom->SetVal("SavePath", 8, value, length);
    }

    void ZSysFile::SetSaveGameName(const char* pszSaveGameName)
    {
        MYSTR saveGameName { pszSaveGameName };
        m_sSaveGameName = saveGameName;

        // TODO: Finish me after CSharedCom will be reversed.
        // const auto length = m_sSaveGameName.Length();
        // const auto* value = m_sSaveGameName.String;
        // CSharedCom::SetVal(static_cast<CSharedCom*>(g_pGlobalCom), "SaveName", 8, value, length);
    }

    void ZSysFile::RestoreSaveGamePath()
    {
        char savePath[128] {};

        // TODO: Finish me after CSharedCom will be reversed.
        // CSharedCom::GetVal(static_cast<CSharedCom*>(g_pGlobalCom), savePath, "SavePath", 8);

        MYSTR restoredSavePath { savePath };
        m_sSaveGamePath = restoredSavePath;
    }

    void ZSysFile::RestoreSaveGameName()
    {
        char saveName[128] {};

        // TODO: Finish me after CSharedCom will be reversed.
        // CSharedCom::GetVal(static_cast<CSharedCom*>(g_pGlobalCom), saveName, "SaveName", 8);

        MYSTR restoredSaveName { saveName };
        m_sSaveGameName = restoredSaveName;
    }

    void ZSysFile::GetFilesInDir(STRREFTAB* paFiles, const char* pDirPath)
    {
        // Nothing here
    }

    uint32_t* ZSysFile::AddBig(const char* pFileName, bool bLoadWhole)
    {
        MYSTR normalizedName { pFileName };
        for (int i = 0; i < normalizedName.Length(); ++i)
        {
            if (normalizedName.String[i] == '\\')
            {
                normalizedName.String[i] = '/';
            }
        }

        MYSTR zipName { normalizedName };
        auto* extension = strrchr(zipName.String, '.');
        if (!extension)
        {
            return nullptr;
        }

        memcpy(extension, ".ZIP", 4);
        if (BigLoaded(zipName.String))
        {
            return nullptr;
        }

        if (!Exists(zipName.String, true))
        {
            printf("ERROR: unable to load big file \"%s\"\n", pFileName);
            return nullptr;
        }

        if (!m_pBigFiles)
        {
            m_pBigFiles = ZUniMemory::New<LINKREFTAB>(4, 1);
        }

        auto* zipfs = ZUniMemory::New<FsZip_t>();
        const auto access = bLoadWhole ? IOFS_PRECACHE : IOFS_READONLY;
        if (!zipfs->initFS(zipName.String, access))
        {
            printf("ERROR: unable to load big file \"%s\"\n", pFileName);
            ZUniMemory::Delete(zipfs);
            return nullptr;
        }

        if (m_pBigFiles)
        {
            for (auto* filesystem : m_pBigFiles->As<IOFilesystem_t*>())
            {
                filesystem->invalidateRedundantFiles(*zipfs);
            }
        }

        return m_pBigFiles->AddEnd(reinterpret_cast<uint32_t>(zipfs));
    }

    const char* ZSysFile::RemoveSysPath(const char* pFileName)
    {
        if (m_pLoadFilter)
        {
            SetFilenameMangling(true);

            const auto* extension = strrchr(pFileName, '.');
            if (!extension)
            {
                return nullptr;
            }

            for (const auto* filter : *m_pLoadFilter)
            {
                if (strchr(filter, '*'))
                {
                    if (WildcardCompareI(pFileName, filter))
                    {
                        return nullptr;
                    }
                }
                else if (stricmp(extension, filter) == 0)
                {
                    SetFilenameMangling(false);
                    return nullptr;
                }
            }
        }

        const auto* systemPath = g_pSysInterface ? g_pSysInterface->m_sSystemPath.String : nullptr;
        if (systemPath && *systemPath)
        {
            const auto systemPathLength = strlen(systemPath);
            if (strlen(pFileName) >= systemPathLength && memicmp(systemPath, pFileName, systemPathLength) == 0)
            {
                pFileName += systemPathLength;
                if (*pFileName == '\\')
                {
                    ++pFileName;
                }
            }
        }

        return pFileName;
    }

    ZSysFile::ZSysFile() = default;
    
    ZSysFile::~ZSysFile()
    {
        RemoveAllBigs();
        if (m_pLoadFilter)
        {
            ZUniMemory::Delete(m_pLoadFilter);
            m_pLoadFilter = nullptr;
        }
    }

    // Static
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZSysFile*, g_pSysFile, 0x0082081C, nullptr);
}
