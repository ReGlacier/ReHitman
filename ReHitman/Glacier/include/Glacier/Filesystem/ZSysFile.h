#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/Filesystem/Fwd.h>
#include <Glacier/ZSTL/LINKREFTAB.h>
#include <Glacier/ZSTL/STRREFTAB.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <cstdint>


namespace Glacier
{
    class ZSysFile
    {
    public:
        // vtbl
        virtual void Restart() = 0;
        virtual void PrintStatus() = 0;
        virtual void* Open(const char* ) = 0;
        virtual void* OpenForAppend(const char* ) = 0;
        virtual void* OpenForRandomAccess(char* ) = 0;
        virtual void* Create(const char* ) = 0;
        virtual bool StartTemp(char* ) = 0;
        virtual void Close(void* ) = 0;
        virtual bool IsLoadedByLoadfilter(const char* pFileName);
        virtual bool GetFileTime(const char* , _FILETIME* , bool) = 0;
        virtual uint64_t GetFileTime64(const char* , bool) = 0;
        virtual uint64_t GetCurrentFileTime() = 0;
        virtual bool FileTimeToSysTime(uint64_t, SSystemTime* ) = 0;
        virtual int GetSize(const char* , bool) = 0;
        virtual bool Exists(const char* , bool) = 0;
        virtual bool WriteTo(void* , void* , int) = 0;
        virtual int ReadFrom(void* File, void* , int, int) = 0;
        virtual int ReadFrom(void* File, void* , int) = 0;
        virtual int Seek(void* , int) = 0;
        virtual void CD(char* ) = 0;
        virtual bool MakeDir(int, int, const char* ) = 0;
        virtual void MakeDir(char* ) = 0;
        virtual void MakeDirNested(const char* DirName);
        virtual void Rename(char* , char* ) = 0;
        virtual void Copy(char* , char* ) = 0;
        virtual bool Delete(const char* ) = 0;
        virtual int Load(const char* , void* , int, int, bool) = 0;
        virtual void Save(const char* , void* , int, int) = 0;
        virtual void Append(const char* , void* , int) = 0;
        virtual void SetFilenameMangling(bool);
        virtual IOFilesystem_t* GetZipFile(const char* pName);
        virtual void PrintBigFiles();
        virtual void SetLoadFilter(const char* pLoadFilter);
        virtual void RemoveLoadFilter(const char* pExtension);
        virtual void CreateListOfBigFileSizes(REFTAB* prtBigFilesSizes);
        virtual void InvalidateZipFile(const char* pFileName);
        virtual void LoadWholeSceneZip(const char* pFileName);
        virtual bool UseBig(CHUNKFILE* pChunkBigFile, const char* pName);
        virtual bool UseBig(const char* pFileName, bool bLoadWhole);
        virtual bool RemoveBig(const char* pFileName);
        virtual void SyncOpenBig(const char* pName);
        virtual void SyncCloseBig(const char* pName);
        virtual bool BigLoaded(const char* pFileName);
        virtual void RemoveAllBigs();
        virtual FsZip_t* CreateZipFile(const char* name);
        virtual void CloseZipFile(IOFilesystem_t* pZfs);
        virtual void AddToZipFile(IOFilesystem_t* pZipfs, const char* name, const void* pData, uint32_t lSize);
        virtual bool CheckOtherZipExt(const char* pName, const char* extList);
        virtual void AddExclusiveAllFiles(CHUNK* pChunk, const char* exclusiveList);
        virtual bool ExistsInZip(const char* pFileName);
        virtual int LoadLib(const char* ) = 0;
        virtual bool FreeLib(int) = 0;
        virtual void GetDllExports(const char* , CHUNK* , CHUNK* ) = 0;
        virtual ~ZSysFile();
        virtual MYSTR ConvertFilename(const char* pszFilename);
        virtual bool PrepareSaveGame(const char* , const char* , char* , int, bool);
        virtual bool SaveGame(void* , uint32_t, uint32_t, uint32_t) = 0;
        virtual bool LoadGame(void* , uint32_t, uint32_t) = 0;
        virtual int GameSize(uint32_t, uint32_t) = 0;
        virtual void RemoveSaveGame(uint32_t) = 0;
        virtual void SetSaveGamePath(const char* pszSaveGamePath);
        virtual void SetSaveGameName(const char* pszSaveGameName);
        virtual void RestoreSaveGamePath();
        virtual void RestoreSaveGameName();
        virtual uint32_t MaxNumPorts() = 0;
        virtual void EnumSaveGames(uint32_t, uint32_t, STRREFTAB* ) = 0;
        virtual void GetFilesInDir(STRREFTAB* paFiles, const char* pDirPath);
        virtual int CreateSaveGame(uint32_t, uint32_t, const char* , const void* , uint32_t, uint32_t* ) = 0;
        virtual int LoadSaveGame(uint32_t, uint32_t, const char* , void* , uint32_t) = 0;
        virtual int SaveGameSize(uint32_t, uint32_t, const char* , uint32_t* ) = 0;
        virtual int DeleteSaveGame(uint32_t, uint32_t, const char* ) = 0;
        virtual int GetStatus(int, int, int* ) = 0;
        virtual uint32_t GetNumSlots(int) = 0;
        virtual bool Format(int, int) = 0;
        virtual bool McFileExists(uint32_t, uint32_t, const char* ) = 0;
        virtual uint32_t MaxNumSlots() = 0;
        virtual bool IsValid(uint32_t, uint32_t) = 0;
        virtual MYSTR NormalizeFilename(const char* ) = 0;
        virtual uint32_t* AddBig(const char* pFileName, bool bLoadWhole);
        virtual MYSTR ConvertFileNameFromZipToHD(const char* ) = 0;
        virtual const char* RemoveSysPath(const char* pFileName);

        // methods
        ZSysFile();
        MYSTR* ConvertFilename(MYSTR* result, const char* pszFilename);

        // members
        LINKREFTAB* m_pBigFiles{nullptr};
        STRREFTAB* m_pLoadFilter{nullptr};
        MYSTR m_sSaveGamePath{};
        MYSTR m_sSaveGameName{};
    };
    RE_VERIFY_SIZE(ZSysFile, 0x10C); // Not verified in PC

    STATIC_GLOBAL_CLASS_INSTANCE(ZSysFile*, g_pSysFile);
}
