#include <Glacier/Filesystem/ZSysFileWintel.h>
#include <Glacier/Filesystem/IOFilesystem_t.h>
#include <Glacier/Filesystem/SSystemTime.h>
#include <Glacier/Filesystem/_FILETIME.h>
#include <Glacier/ZSTL/CHUNK.h>
#include <Glacier/System/ZSysInterface.h>

#include <Windows.h>
#include <cstdio>


namespace Glacier
{
    #pragma pack(push, 1)
    struct ZSectionEntry
    {
        uint32_t headerMarker;
        uint8_t  padding[8];
        uint32_t startOffset;
        uint32_t size;
        uint32_t fileOffset;
        uint8_t  reserved[16];
    };
    #pragma pack(pop)
    RE_VERIFY_SIZE(ZSectionEntry, 0x28);

    struct LoadedLibrary_t
    {
        char* name;
        HMODULE module;
        uint32_t refCount;
        void* classInfoChunk;
        void* procNamesChunk;
    };
    RE_VERIFY_SIZE(LoadedLibrary_t, 0x14);

    bool EndsWith(const char* str, const char* suffix)
    {
        const auto strLength = strlen(str);
        const auto suffixLength = strlen(suffix);
        return strLength >= suffixLength && strcmp(str + strLength - suffixLength, suffix) == 0;
    }

    ZSysFileWintel::ZSysFileWintel()
        : ZSysFile()
    {
        m_prtLibraries = ZUniMemory::New<LINKREFTAB>(32, 4);
        g_pSysFile = this;
    }

    ZSysFileWintel::~ZSysFileWintel()
    {
        if (m_prtLibraries)
        {
            for (auto* record : m_prtLibraries->As<LoadedLibrary_t*>())
            {
                if (record->classInfoChunk)
                {
                    ZUniMemory::Delete(record->classInfoChunk);
                }

                if (record->procNamesChunk)
                {
                    ZUniMemory::Delete(record->procNamesChunk);
                }

                printf("Freeing library '%s'\n", record->name);
                ZUniMemory::Free(record->name);
                FreeLibrary(record->module);
            }

            ZUniMemory::Delete(m_prtLibraries);
            m_prtLibraries = nullptr;
        }

        g_pSysFile = nullptr;
    }

    void ZSysFileWintel::Restart()
    {
        // Do nothing
    }

    void ZSysFileWintel::PrintStatus()
    {
        // Do nothing
    }

    void* ZSysFileWintel::Open(const char* FileName)
    {
        HANDLE hFile = CreateFileA( FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        return (hFile == INVALID_HANDLE_VALUE) ? nullptr : (void*)hFile;
    }

    void* ZSysFileWintel::OpenForAppend(const char* FileName)
    {
        HANDLE hFile = CreateFileA(FileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            return nullptr;
        }

        SetFilePointer(hFile, 0, NULL, FILE_END);
        return (void*)hFile;
    }

    void* ZSysFileWintel::OpenForRandomAccess(char* FileName)
    {
        HANDLE hFile = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            return NULL;
        }

        return (void*)hFile;
    }

    void* ZSysFileWintel::Create(const char* FileName)
    {
        HANDLE hFile = CreateFileA(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD errorCode = GetLastError();
            char errorBuffer[256];

            DWORD formatLen = FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                errorBuffer,
                sizeof(errorBuffer),
                NULL
            );

            if (formatLen > 0)
            {
                errorBuffer[strcspn(errorBuffer, "\r\n")] = 0;
                printf("[ZSysFileWintel::Create] Failed to create file '%s': %s (Code: %lu)\n",  FileName, errorBuffer, errorCode);
            }
            else
            {
                printf("[ZSysFileWintel::Create] Failed to create file '%s': Unknown error (Code: %lu)\n", FileName, errorCode);
            }

            return nullptr;
        }

        return hFile;
    }
    
    bool ZSysFileWintel::StartTemp(char* FileName)
    {
        HANDLE hFile = CreateFileA(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        
        Close((void*)hFile);
        return true;
    }
    
    void ZSysFileWintel::Close(void* Handle)
    {
        CloseHandle((HANDLE)Handle);
    }

    bool ZSysFileWintel::GetFileTime(const char* pFileName, Glacier::_FILETIME* pFileTime, bool NoBig)
    {
        if (!NoBig)
        {
            const auto* zipFileName = RemoveSysPath(pFileName);
            if (zipFileName)
            {
                auto* zipFile = GetZipFile(zipFileName);
                if (zipFile && zipFile->GetFileTime(zipFileName, pFileTime))
                {
                    return true;
                }
            }
        }

        auto* Handle = Open(pFileName);
        if (!Handle)
            return false;

        ::_FILETIME LastAccessTime, CreationTime, FileTime;
        const auto result = ::GetFileTime((HANDLE)Handle, &CreationTime, &LastAccessTime, &FileTime);

        Close(Handle);
        if (!result)
        {
            return false;
        }
        
        pFileTime->dwHighDateTime = FileTime.dwHighDateTime;
        pFileTime->dwLowDateTime = FileTime.dwLowDateTime;
        
        return true;
    }
    
    uint64_t ZSysFileWintel::GetFileTime64(const char* pFileName, bool NoBig)
    {
        Glacier::_FILETIME time;

        if (GetFileTime(pFileName, &time, NoBig))
        {
            return static_cast<uint64_t>(time);
        }

        return 0u;
    }

    uint64_t ZSysFileWintel::GetCurrentFileTime()
    {
        ::_FILETIME time {};

        GetSystemTimeAsFileTime(&time);
        
        Glacier::_FILETIME ourFileTime { time.dwLowDateTime, time.dwHighDateTime };
        return static_cast<uint64_t>(ourFileTime);
    }
    
    bool ZSysFileWintel::FileTimeToSysTime(uint64_t iFileTime, SSystemTime* pSysTime)
    {
        ::_FILETIME fileTime, localFileTime;
        ::_SYSTEMTIME systemTime;

        if (!FileTimeToLocalFileTime(&fileTime, &localFileTime))
        {
            return false;
        }

        if (!FileTimeToSystemTime(&fileTime, &systemTime))
        {
            return false;
        }

        pSysTime->wYear = systemTime.wYear;
        pSysTime->wMonth = systemTime.wMonth;
        pSysTime->wDayOfWeek = systemTime.wDayOfWeek;
        pSysTime->wDay = systemTime.wDay;
        pSysTime->wHour = systemTime.wHour;
        pSysTime->wMinute = systemTime.wMinute;
        pSysTime->wSecond = systemTime.wSecond;
        pSysTime->wMilliseconds = systemTime.wMilliseconds;

        return true;
    }

    int ZSysFileWintel::GetSize(const char* pFileName, bool NoBig)
    {
        if (!NoBig)
        {
            const auto zipFileName = ConvertFileNameFromZipToHD(pFileName);
            if (zipFileName)
            {
                auto* zipFile = GetZipFile(zipFileName);
                if (zipFile)
                {
                    const auto size = zipFile->GetSize(zipFileName);
                    if (size != -1)
                    {
                        return size;
                    }
                }
            }
        }

        auto* handle = Open(pFileName);
        if (!handle)
        {
            return -1;
        }

        const auto size = GetFileSize(static_cast<HANDLE>(handle), nullptr);
        Close(handle);

        return static_cast<int>(size);
    }
    
    bool ZSysFileWintel::Exists(const char* pFileName, bool NoBig)
    {
        if (!NoBig)
        {
            const auto zipFileName = ConvertFileNameFromZipToHD(pFileName);
            if (zipFileName)
            {
                auto* zipFile = GetZipFile(zipFileName);
                if (zipFile && zipFile->Exists(zipFileName))
                {
                    return true;
                }
            }
        }

        WIN32_FIND_DATAA findFileData;
        const auto handle = FindFirstFileA(pFileName, &findFileData);
        if (handle == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        FindClose(handle);
        return true;
    }

    bool ZSysFileWintel::WriteTo(void* WriteTo, void* Ptr, int Size)
    {
        return WriteFile(WriteTo, Ptr, Size, (LPDWORD)&Size, 0);
    }

    int ZSysFileWintel::ReadFrom(void* File, void* Ptr, int Size, int lOffset)
    {
        // It's really weird, because in Xenon there are looks like
        // SetFilePointer(WriteTo, lOffset, 0, 0);
        // return ReadFrom(WriteTo, Ptr, Size);
        //
        // But in Hitman Blood Money, PC, at function 00593F70 we have only this part
        // Same on iOS
        return 0;
    }

    int ZSysFileWintel::ReadFrom(void* hFile, void* pBuffer, int lSize)
    {
        DWORD bytesRead = 0;

        if (!ReadFile(static_cast<HANDLE>(hFile), pBuffer, static_cast<DWORD>(lSize), &bytesRead, NULL))
        {
            DWORD lastError = GetLastError();
            LPSTR errorText = nullptr;

            constexpr DWORD lFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

            FormatMessageA(lFlags, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&errorText), 0, NULL);

            if (errorText)
            {
                MessageBoxA(NULL, errorText, "Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);
                LocalFree(errorText);
            }
        }

        return static_cast<int32_t>(bytesRead);
    }

    int ZSysFileWintel::Seek(void* Handle, int iOffset)
    {
        return static_cast<int>(SetFilePointer((HANDLE)Handle, iOffset, 0, 0));
    }
    
    void ZSysFileWintel::CD(char* DirName)
    {
        SetCurrentDirectoryA(DirName);
    }
    
    bool ZSysFileWintel::MakeDir(int, int, const char* DirName)
    {
        return false;
    }

    void ZSysFileWintel::MakeDir(char* DirName)
    {
        char aBuffer[260] { 0 };
        char aErrorMessageBuffer[1024] { 0 };

        const auto lPathLen = strlen(DirName);
        if (lPathLen < sizeof(aBuffer))
        {
            strcpy(aBuffer, DirName);

            for (auto& ch : aBuffer)
            {
                if (ch == '\\')
                    ch = '/';
            }

            // Dir not exists?
            if (!Exists(aBuffer, false))
            {
                char* pLastSlash = strchr(aBuffer, '/');

                if (pLastSlash)
                {
                    *pLastSlash = '\0';
                    MakeDir(aBuffer);
                    *pLastSlash = '/';
                }

                if (!CreateDirectoryA(aBuffer, NULL))
                {
                    DWORD lLastError = GetLastError();

                    FormatMessageA(
                        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        lLastError,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        aErrorMessageBuffer,
                        sizeof(aErrorMessageBuffer),
                        NULL
                    );

                    printf(
                        "[ZSysFileWintel::MakeDir] Failed to create directory '%s': %s (Code: %lu)\n",
                        aBuffer, aErrorMessageBuffer, lLastError
                    );
                }
            }
        }
    }

    void ZSysFileWintel::Rename(char* SrcName, char* DstName)
    {
        MoveFileA(SrcName, DstName);
    }

    void ZSysFileWintel::Copy(char* SrcPath, char* DstPath)
    {
        CopyFileA(SrcPath, DstPath, true);
    }

    bool ZSysFileWintel::Delete(const char* FileName)
    {
        return DeleteFileA(FileName);
    }

    int ZSysFileWintel::Load(const char* pFileName, void* Ptr, int Size, int Offset, bool NoBig)
    {
        if (!NoBig)
        {
            auto zipFileName = ConvertFileNameFromZipToHD(pFileName);
            auto* zipFile = GetZipFile(zipFileName.String);
            if (zipFile)
            {
                const auto bytesRead = zipFile->Load(zipFileName.String, Ptr, Size, Offset);
                if (bytesRead != -1)
                {
                    return bytesRead;
                }
            }
        }

        if (Offset)
        {
            auto* file = fopen(pFileName, "rb");
            if (file)
            {
                fseek(file, Offset, SEEK_SET);
                const auto bytesRead = fread(Ptr, 1, Size, file);
                fclose(file);
                return static_cast<int>(bytesRead);
            }
        }
        else
        {
            auto* handle = Open(pFileName);
            if (handle)
            {
                const auto bytesRead = ReadFrom(handle, Ptr, Size);
                Close(handle);
                return bytesRead;
            }
        }

        return -1;
    }

    void ZSysFileWintel::Save(const char* FileName, void* Ptr, int Size, int lWriteSize)
    {
        auto* handle = Create(FileName);
        if (!handle)
        {
            return;
        }

        if (lWriteSize && Size > lWriteSize)
        {
            WriteTo(handle, Ptr, lWriteSize);
            Close(handle);

            auto* appendHandle = OpenForAppend(FileName);
            auto* data = static_cast<char*>(Ptr) + lWriteSize;
            auto fullChunksRemaining = Size / lWriteSize - 1;

            while (fullChunksRemaining-- > 0)
            {
                WriteTo(appendHandle, data, lWriteSize);
                data += lWriteSize;
            }

            const auto remainder = Size % lWriteSize;
            if (remainder)
            {
                WriteTo(appendHandle, data, remainder);
            }

            Close(appendHandle);
        }
        else
        {
            WriteTo(handle, Ptr, Size);
            Close(handle);
        }
    }
    
    void ZSysFileWintel::Append(const char* FileName, void* Ptr, int Size)
    {
        auto* handle = OpenForAppend(FileName);
        if (!handle) return;

        WriteTo(handle, Ptr, Size);
        Close(handle);
    }
    
    int ZSysFileWintel::LoadLib(const char* pszLib)
    {
        auto* libraryName = strrchr(pszLib, '\\');
        if (!libraryName)
        {
            libraryName = const_cast<char*>(pszLib - 1);
        }

        ++libraryName;

        RefRun it;
        m_prtLibraries->RunInitNxtRef(&it);
        for (auto* record = reinterpret_cast<LoadedLibrary_t*>(m_prtLibraries->RunNxtRefPtr(&it)); record; record = reinterpret_cast<LoadedLibrary_t*>(m_prtLibraries->RunNxtRefPtr(&it)))
        {
            if (stricmp(record->name, libraryName) == 0)
            {
                ++record->refCount;
                return reinterpret_cast<int>(record->module);
            }
        }

        auto module = LoadLibraryA(pszLib);
        auto* storedName = static_cast<char*>(ZUniMemory::Allocate(strlen(libraryName) + 1));
        strcpy(storedName, libraryName);

        auto* record = reinterpret_cast<LoadedLibrary_t*>(m_prtLibraries->Add(reinterpret_cast<uint32_t>(storedName)) - 1);
        record->module = module;
        record->refCount = 1;
        record->classInfoChunk = nullptr;
        record->procNamesChunk = nullptr;

        return reinterpret_cast<int>(module);
    }

    bool ZSysFileWintel::FreeLib(int hLib)
    {
        if (!m_prtLibraries)
        {
            return false;
        }

        RefRun it;
        m_prtLibraries->RunInitNxtRef(&it);
        for (auto* record = reinterpret_cast<LoadedLibrary_t*>(m_prtLibraries->RunNxtRefPtr(&it)); record; record = reinterpret_cast<LoadedLibrary_t*>(m_prtLibraries->RunNxtRefPtr(&it)))
        {
            if (reinterpret_cast<HMODULE>(hLib) != record->module)
            {
                continue;
            }

            if (record->refCount-- != 1)
            {
                return false;
            }

            if (record->classInfoChunk)
            {
                ZUniMemory::Delete(record->classInfoChunk);
            }

            if (record->procNamesChunk)
            {
                ZUniMemory::Delete(record->procNamesChunk);
            }

            ZUniMemory::Free(record->name);
            m_prtLibraries->RunDelRef(&it);
            FreeLibrary(record->module);

            return true;
        }

        return false;
    }

    void ZSysFileWintel::GetDllExports(const char* pLibraryFileName, CHUNK* pClassInfoChunk, CHUNK* pProcNamesChunk)
    {
        LoadDllExports(pLibraryFileName, pClassInfoChunk, pProcNamesChunk);
    }

    bool ZSysFileWintel::SaveGame(void* pData, uint32_t iSize, uint32_t iSaveGameNumber, uint32_t iSaveGamePart)
    {
        char aSaveFileNameBuffer[1024] { 0 };
        CreateSaveGameName(aSaveFileNameBuffer, iSaveGameNumber, iSaveGamePart);

        char* psSaveGamePath = m_sSaveGamePath;
        MakeDir(psSaveGamePath);

        Save(aSaveFileNameBuffer, pData, iSize, 0);
        return GetSize(aSaveFileNameBuffer, false) == iSize;
    }

    bool ZSysFileWintel::LoadGame(void* pData, uint32_t iSaveGameNumber, uint32_t iSaveGamePart)
    {
        char aSaveFileName[1024] { 0 };

        CreateSaveGameName(aSaveFileName, iSaveGameNumber, iSaveGamePart);
        int lSize = GameSize(iSaveGameNumber, iSaveGamePart);
        return lSize != -1 && Load(aSaveFileName, pData, lSize, 0, false) != -1;
    }

    int ZSysFileWintel::GameSize(uint32_t iSaveGameNumber, uint32_t iSaveGamePart)
    {
        char aSaveFileName[1024] { 0 };

        CreateSaveGameName(aSaveFileName, iSaveGameNumber, iSaveGamePart);
        return GetSize(aSaveFileName, false);
    }

    void ZSysFileWintel::RemoveSaveGame(uint32_t iSaveGameNumber)
    {
        char aSaveNameBuffer[1024] { 0 };

        for (int i = 1; i < 4; ++i)
        {
            CreateSaveGameName(aSaveNameBuffer, iSaveGameNumber, i);
            Delete(aSaveNameBuffer);
        }
    }

    void ZSysFileWintel::EnumSaveGames(uint32_t iSaveGameNumber, uint32_t iSaveGamePart, STRREFTAB* pFound)
    {
        ZASSERT(!iSaveGameNumber && !iSaveGamePart);

        char fileName[260] {};
        strcpy(fileName, static_cast<const char*>(g_pSysInterface->m_sProjectPath));

        strcat(fileName, "Savegame\\");
        MakeDir(fileName);

        strcat(fileName, "*");

        WIN32_FIND_DATAA findFileData;
        const auto handle = FindFirstFileA(fileName, &findFileData);
        if (handle == INVALID_HANDLE_VALUE)
        {
            return;
        }

        do
        {
            if (findFileData.cFileName[0] != '.' || (findFileData.cFileName[1] && (findFileData.cFileName[1] != '.' || findFileData.cFileName[2])))
            {
                pFound->AddStr(findFileData.cFileName);
            }
        }
        while (FindNextFileA(handle, &findFileData));

        FindClose(handle);
    }
    
    void ZSysFileWintel::GetFilesInDir(STRREFTAB* paFiles, const char* pDirPath)
    {
        char fileName[260] {};
        strcpy(fileName, static_cast<const char*>(g_pSysInterface->m_sProjectPath));
        strcat(fileName, pDirPath);

        if (!Exists(fileName, false))
        {
            MakeDir(fileName);
        }

        strcat(fileName, "\\");
        strcat(fileName, "*");

        WIN32_FIND_DATAA findFileData;
        const auto handle = FindFirstFileA(fileName, &findFileData);
        if (handle == INVALID_HANDLE_VALUE)
        {
            return;
        }

        do
        {
            if (findFileData.cFileName[0] != '.' || (findFileData.cFileName[1] && (findFileData.cFileName[1] != '.' || findFileData.cFileName[2])))
            {
                paFiles->AddStr(findFileData.cFileName);
            }
        }
        while (FindNextFileA(handle, &findFileData));

        FindClose(handle);
    }

    int ZSysFileWintel::CreateSaveGame(uint32_t iSaveGameNumber, uint32_t iSaveGamePart, const char* psName, const void* pData, uint32_t size, uint32_t*)
    {
        ZASSERT(!iSaveGameNumber && !iSaveGamePart);

        char pathName[260] {};
        strcpy(pathName, static_cast<const char*>(g_pSysInterface->m_sProjectPath));

        strcat(pathName, "Savegame\\");
        MakeDir(pathName);

        strcat(pathName, psName);
        MakeDir(pathName);

        char saveDataPath[260] {};
        strcpy(saveDataPath, pathName);
        strcat(saveDataPath, "\\");
        strcat(saveDataPath, "savedata.bin");

        Save(saveDataPath, const_cast<void*>(pData), size, 0);

        return 0;
    }
    
    int ZSysFileWintel::LoadSaveGame(uint32_t iSaveGameNumber, uint32_t iSaveGamePart, const char* psName, void*, uint32_t)
    {
        ZASSERT(!iSaveGameNumber && !iSaveGamePart);

        char pathName[260] {};
        strcpy(pathName, static_cast<const char*>(g_pSysInterface->m_sProjectPath));

        strcat(pathName, "Savegame\\");
        MakeDir(pathName);

        strcat(pathName, psName);
        MakeDir(pathName);

        char saveDataPath[260] {};
        strcpy(saveDataPath, pathName);
        strcat(saveDataPath, "\\");
        strcat(saveDataPath, "savedata.bin");

        GetSize(saveDataPath, false);

        return 0;
    }

    int ZSysFileWintel::SaveGameSize(uint32_t iSaveGameNumber, uint32_t iSaveGamePart, const char* psName, uint32_t* pSize)
    {
        ZASSERT(!iSaveGameNumber && !iSaveGamePart);

        char pathName[260] {};
        strcpy(pathName, static_cast<const char*>(g_pSysInterface->m_sProjectPath));

        strcat(pathName, "Savegame\\");
        MakeDir(pathName);

        strcat(pathName, psName);
        MakeDir(pathName);

        char saveDataPath[260] {};
        strcpy(saveDataPath, pathName);
        strcat(saveDataPath, "\\");
        strcat(saveDataPath, "savedata.bin");

        *pSize = GetSize(saveDataPath, false);

        return 0;
    }

    int ZSysFileWintel::DeleteSaveGame(uint32_t, uint32_t, const char* psName)
    {
        char pathName[260] {};
        strcpy(pathName, static_cast<const char*>(g_pSysInterface->m_sProjectPath));

        strcat(pathName, "Savegame\\");
        MakeDir(pathName);

        strcat(pathName, psName);
        MakeDir(pathName);

        char saveDataPath[260] {};
        strcpy(saveDataPath, pathName);
        strcat(saveDataPath, "\\");
        strcat(saveDataPath, "savedata.bin");

        Delete(saveDataPath);
        RemoveDirectoryA(pathName);

        return 0;
    }

    MYSTR ZSysFileWintel::ConvertFileNameFromZipToHD(const char* pFileName)
    {
        MYSTR result = g_pSysInterface->m_sSystemPath;

        result += "\\";
        result += pFileName;

        return result;
    }
    
    void ZSysFileWintel::EnumSaveGames(STRREFTAB* pFound, const char* pDirPath)
    {
        GetFilesInDir(pFound, pDirPath);
    }
    
    int32_t ZSysFileWintel::GetFilePos(void* vfile, int sectPos, uint32_t NamePos)
    {
        // NOTE: I'm really not sure about this method, will ckeck it later
        if (!vfile)
        {
            return 0;
        }

        FILE* pFile = static_cast<FILE*>(vfile);
        int currentEntryPos = sectPos;

        uint32_t marker = 0;
        std::fseek(pFile, currentEntryPos, SEEK_SET);
        std::fread(&marker, sizeof(uint32_t), 1, pFile);

        if (marker == 0)
        {
            return 0;
        }

        while (true)
        {
            std::fseek(pFile, currentEntryPos + 0xC, SEEK_SET);

            uint32_t rangeStart = 0;
            uint32_t rangeSize = 0;
            uint32_t fileDataOffset = 0;

            std::fread(&rangeStart,     sizeof(uint32_t), 1, pFile);
            std::fread(&rangeSize,      sizeof(uint32_t), 1, pFile);
            std::fread(&fileDataOffset, sizeof(uint32_t), 1, pFile);

            if (NamePos >= rangeStart && NamePos < (rangeStart + rangeSize))
            {
                return static_cast<int32_t>(NamePos + fileDataOffset - rangeStart);
            }

            currentEntryPos += 40;

            std::fseek(pFile, currentEntryPos, SEEK_SET);
            std::fread(&marker, sizeof(uint32_t), 1, pFile);

            if (marker == 0)
                break;
        }

        return 0;
    }

    void ZSysFileWintel::LoadDllExports(const char* pLibraryFileName, CHUNK* pClassInfoChunk, CHUNK* pProcNamesChunk)
    {
        // Resolve the actual DLL path. Glacier.dlc is the main executable in this build.
        MYSTR libraryPath;
        if (stricmp(pLibraryFileName, "Glacier.dlc") == 0)
        {
            char filename[MAX_PATH] {};
            GetModuleFileNameA(reinterpret_cast<HMODULE>(g_pSysInterface->m_hInstance), filename, sizeof(filename));
            libraryPath = MYSTR { filename };
        }
        else if (strchr(pLibraryFileName, '\\'))
        {
            libraryPath = MYSTR { pLibraryFileName };
        }
        else
        {
            // Bare DLL names are resolved relative to the engine system directory.
            libraryPath = g_pSysInterface->m_sSystemPath;
            libraryPath += "\\";
            libraryPath += pLibraryFileName;
        }

        auto* file = fopen(libraryPath.String, "rb");
        if (!file)
        {
            return;
        }

        int32_t peOffset = 0;
        // DOS header e_lfanew field: file offset of the PE header.
        fseek(file, 0x3C, SEEK_SET);
        fread(&peOffset, 1, sizeof(peOffset), file);

        uint32_t peSignature = 0;
        fseek(file, peOffset, SEEK_SET);
        fread(&peSignature, 1, sizeof(peSignature), file);
        // PE signature is "PE\0\0" stored as little-endian 0x4550.
        if (peSignature == 0x4550)
        {
            uint16_t optionalHeaderSize = 0;
            // IMAGE_FILE_HEADER.SizeOfOptionalHeader is 20 bytes after PE header start.
            fseek(file, peOffset + 20, SEEK_SET);
            fread(&optionalHeaderSize, 1, sizeof(optionalHeaderSize), file);

            // Section headers start after signature (4), file header (20), and optional header.
            const auto sectionTableOffset = peOffset + 24 + optionalHeaderSize;

            uint32_t exportDirectoryRva = 0;
            // Optional header data directory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress.
            // In PE32 this is 96 bytes from the optional header start.
            fseek(file, peOffset + 24 + 96, SEEK_SET);
            fread(&exportDirectoryRva, 1, sizeof(exportDirectoryRva), file);
            if (!exportDirectoryRva)
            {
                fclose(file);
                return;
            }

            uint32_t exportDirectorySize = 0;
            fread(&exportDirectorySize, 1, sizeof(exportDirectorySize), file);

            // Convert export directory RVA to a raw file offset using the section table.
            const auto exportDirectoryOffset = GetFilePos(file, sectionTableOffset, exportDirectoryRva);

            uint32_t numberOfNames = 0;
            // IMAGE_EXPORT_DIRECTORY.NumberOfNames at offset 0x18.
            fseek(file, exportDirectoryOffset + 24, SEEK_SET);
            fread(&numberOfNames, 1, sizeof(numberOfNames), file);

            uint32_t addressOfNamesRva = 0;
            // IMAGE_EXPORT_DIRECTORY.AddressOfNames at offset 0x20.
            fseek(file, exportDirectoryOffset + 32, SEEK_SET);
            fread(&addressOfNamesRva, 1, sizeof(addressOfNamesRva), file);
            const auto addressOfNamesOffset = GetFilePos(file, sectionTableOffset, addressOfNamesRva);

            char exportName[102400] {};
            for (uint32_t i = 0; i < numberOfNames; ++i)
            {
                // AddressOfNames points to an array of RVAs, one for each exported symbol name.
                uint32_t nameRva = 0;
                fseek(file, addressOfNamesOffset + sizeof(nameRva) * i, SEEK_SET);
                fread(&nameRva, 1, sizeof(nameRva), file);

                const auto nameOffset = GetFilePos(file, sectionTableOffset, nameRva);
                fseek(file, nameOffset, SEEK_SET);
                fscanf(file, "%102399s", exportName);

                // Class info exports are decorated names ending with "_ClassInfo" before '@'.
                auto* at = strchr(exportName, '@');
                if (at && at - exportName > 10 && strncmp(at - 10, "_ClassInfo", 10) == 0)
                {
                    pClassInfoChunk->AddString(exportName);
                }
                // Procedure exports are plain C-style names ending with "_ExpFunc".
                else if (pProcNamesChunk && exportName[0] != '?' && EndsWith(exportName, "_ExpFunc"))
                {
                    pProcNamesChunk->AddString(exportName);
                }
            }
        }

        fclose(file);
    }
    
    uint64_t ZSysFileWintel::GetFreeDiskSpace(const char* szDirectory)
    {
        ULARGE_INTEGER freeBytesAvailable;
        ULARGE_INTEGER totalBytes;
        ULARGE_INTEGER totalFreeBytes;

        if (GetDiskFreeSpaceExA(szDirectory, &freeBytesAvailable, &totalBytes, &totalFreeBytes))
        {
            return freeBytesAvailable.QuadPart;
        }

        return 0u;
    }

    void ZSysFileWintel::CreateSaveGameName(char* psName, uint32_t iSaveGameNumber, uint32_t iSaveGamePart)
    {
        if (!m_sSaveGamePath.Length())
        {
            m_sSaveGamePath = g_pSysInterface->SaveGamePath();
        }

        auto* sysInterface = g_pSysInterface->BeforeFormat();
        sysInterface->SPrintF(
            psName,
            "%s\\%s%d.%d",
            static_cast<const char*>(m_sSaveGamePath),
            static_cast<const char*>(m_sSaveGameName),
            iSaveGameNumber,
            iSaveGamePart);
    }
}
