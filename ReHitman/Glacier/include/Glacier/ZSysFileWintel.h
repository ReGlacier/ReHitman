#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Win32FWD.h>

namespace Glacier
{
    class ZSysFileWintel
    {
    public:
        virtual void Restart();
        virtual void PrintStatus();
        virtual void* Open(const char*);
        virtual void* OpenForAppend(const char*);
        virtual void* OpenForRandomAccess(const char*);
        virtual void* Create(const char*);
        virtual void* StartTemp(const char*);
        virtual void Close(void*);
        virtual bool IsLoadedByLoadfilter(const char*);
        virtual bool GetFileTime(const char*, FILETIME*, bool);
        virtual bool GetFileTime64(const char*, bool);
        virtual uint64_t GetCurrentFileTime();
        virtual bool FileTimeToSysTime(FILETIME fileTime, SYSTEMTIME* systemtime);
        virtual int32_t GetSize(const char*, bool);
        virtual bool Exists(const char*, bool);
        virtual bool WriteTo(void*, void*, long);
        virtual bool ReadFrom(void*, void*, long);
        virtual bool ReadFrom(void*, void*, long, int);
        virtual void Seek(void*, long);
        virtual void CD(const char*);
        virtual bool MakeDir(const char*);
        virtual bool MakeDirNested(const char*);
        virtual void Rename(const char*, const char*);
        virtual void Move(const char*, const char*);
        virtual void Copy(const char*, const char*);
        virtual void Delete(const char*, const char*);
        virtual int32_t Load(const char*, void*, long, long, bool);
        virtual void Save(const char*, void*, long, long);
        virtual void Append(const char*, void*, long);
        virtual void SetFilenameMangling(bool); // Do nothing
        virtual FsZip_t* GetZipFile(const char*);
        virtual void PrintBigFiles();
        virtual void SetLoadFilter(const char*);
        virtual void RemoveLoadFilter(const char*);
        virtual int CreateListOfBigFileSizes(REFTAB*);
        virtual void InvalidateZipFile(const char*);
        virtual void* LoadWholeSceneZip(const char*);
        virtual void UseBig(const char*, bool);
        virtual void UseBig(CHUNKFILE*, const char*);
        virtual void RemoveBig(const char*);
        virtual void SyncOpenBig(const char*);
        virtual void SyncCloseBig(const char*);
        virtual void BigLoaded(const char*);
        virtual void RemoveAllBigs();
        virtual FsZip_t* CreateZipFile(const char*);
        virtual void CloseZipFile(FsZip_t*);
        virtual void AddToZipFile(FsZip_t*, const char*, const void*, uint32_t);
        virtual bool CheckOtherZipExt(const char*, const char*);
        virtual void AddExclusiveAllFiles(CHUNK*, const char*);
        virtual bool ExistsInZip(const char*);
        virtual long LoadLib(const char*);
        virtual void FreeLib(long);
        virtual void* GetDllExports(const char*, CHUNK*, CHUNK*);
        /*
         *  PS2 Stuff, not ported yet
        .data:00569DE0                 .word _ZN11ZSysFilePS2D1Ev  # ZSysFilePS2::~ZSysFilePS2()
        .data:00569DE4                 .word _ZN11ZSysFilePS2D0Ev  # ZSysFilePS2::~ZSysFilePS2()
        .data:00569DE8                 .word _ZN11ZSysFilePS215ConvertFilenameEPKc  # ZSysFilePS2::ConvertFilename(char const*)
        .data:00569DEC                 .word _ZN8ZSysFile15PrepareSaveGameEPKcS1_Pcib  # ZSysFile::PrepareSaveGame(char const*,char const*,char *,int,bool)
        .data:00569DF0                 .word _ZN11ZSysFilePS28SaveGameEPvjjj  # ZSysFilePS2::SaveGame(void *,uint,uint,uint)
        .data:00569DF4                 .word _ZN11ZSysFilePS28LoadGameEPvjj  # ZSysFilePS2::LoadGame(void *,uint,uint)
        .word _ZN11ZSysFilePS28GameSizeEjj  # ZSysFilePS2::GameSize(uint,uint)
        .data:00569DFC                 .word _ZN11ZSysFilePS214RemoveSaveGameEj  # ZSysFilePS2::RemoveSaveGame(uint)
        .data:00569E00                 .word _ZN8ZSysFile15SetSaveGamePathEPKc  # ZSysFile::SetSaveGamePath(char const*)
        .data:00569E04                 .word _ZN8ZSysFile15SetSaveGameNameEPKc  # ZSysFile::SetSaveGameName(char const*)
        .data:00569E08                 .word _ZN8ZSysFile19RestoreSaveGamePathEv  # ZSysFile::RestoreSaveGamePath(void)
        .data:00569E0C                 .word _ZN8ZSysFile19RestoreSaveGameNameEv  # ZSysFile::RestoreSaveGameName(void)
        .data:00569E10                 .word _ZNK11ZSysFilePS211MaxNumPortsEv  # ZSysFilePS2::MaxNumPorts(void)
        .data:00569E14                 .word _ZN11ZSysFilePS213EnumSaveGamesEjjP9STRREFTAB  # ZSysFilePS2::EnumSaveGames(uint,uint,STRREFTAB *)
        .data:00569E18                 .word _ZN8ZSysFile13GetFilesInDirEP9STRREFTABPKc  # ZSysFile::GetFilesInDir(STRREFTAB *,char const*)
        .data:00569E1C                 .word _ZN11ZSysFilePS214CreateSaveGameEjjPKcPKvjPj  # ZSysFilePS2::CreateSaveGame(uint,uint,char const*,void const*,uint,uint *)
        .data:00569E20                 .word _ZN11ZSysFilePS212LoadSaveGameEjjPKcPvj  # ZSysFilePS2::LoadSaveGame(uint,uint,char const*,void *,uint)
        .data:00569E24                 .word _ZN11ZSysFilePS212SaveGameSizeEjjPKcPj  # ZSysFilePS2::SaveGameSize(uint,uint,char const*,uint *)
        .data:00569E28                 .word _ZN11ZSysFilePS214DeleteSaveGameEjjPKc  # ZSysFilePS2::DeleteSaveGame(uint,uint,char const*)
        .data:00569E2C                 .word _ZN11ZSysFilePS29GetStatusEiiPi  # ZSysFilePS2::GetStatus(int,int,int *)
        .data:00569E30                 .word _ZNK11ZSysFilePS211GetNumSlotsEi  # ZSysFilePS2::GetNumSlots(int)
        .data:00569E34                 .word _ZN11ZSysFilePS26FormatEii  # ZSysFilePS2::Format(int,int)
        .data:00569E38                 .word _ZN11ZSysFilePS27MakeDirEiiPKc  # ZSysFilePS2::MakeDir(int,int,char const*)
        .data:00569E3C                 .word _ZNK11ZSysFilePS212McFileExistsEjjPKc  # ZSysFilePS2::McFileExists(uint,uint,char const*)
        .data:00569E40                 .word _ZNK11ZSysFilePS211MaxNumSlotsEv  # ZSysFilePS2::MaxNumSlots(void)
        .data:00569E44                 .word _ZN11ZSysFilePS27IsValidEjj  # ZSysFilePS2::IsValid(uint,uint)
        .data:00569E48                 .word _ZN11ZSysFilePS26AddBigEPKcb  # ZSysFilePS2::AddBig(char const*,bool)
        .data:00569E4C                 .word _ZN11ZSysFilePS226ConvertFileNameFromZipToHDEPKc  # ZSysFilePS2::ConvertFileNameFromZipToHD(char const*)
        .data:00569E50                 .word _ZN8ZSysFile13RemoveSysPathEPKc  # ZSysFile::RemoveSysPath(char const*)
        .data:00569E54                 .word _ZN11ZSysFilePS210GetFilePosEPvim  # ZSysFilePS2::GetFilePos(void *,int,ulong)
        .data:00569E58                 .word _ZN11ZSysFilePS214LoadDllExportsEPKcP5CHUNKS3_  # ZSysFilePS2::LoadDllExports(char const*,CHUNK *,CHUNK *)
        .data:00569E5C                 .word _ZN11ZSysFilePS28UnformatEii  # ZSysFilePS2::Unformat(int,int)
        .data:00569E60                 .word _ZN11ZSysFilePS28FileSizeEjjPKc  # ZSysFilePS2::FileSize(uint,uint,char const*)
        .data:00569E64                 .word _ZN11ZSysFilePS218GetDirSizeInBlocksEjjPKcPi  # ZSysFilePS2::GetDirSizeInBlocks(uint,uint,char const*,int *)
        .data:00569E68                 .word _ZN11ZSysFilePS219SetPortSlotSaveGameEii  # ZSysFilePS2::SetPortSlotSaveGame(int,int)
        .data:00569E6C                 .word _ZN11ZSysFilePS213CreatePadFileEj  # ZSysFilePS2::CreatePadFile(uint)
        .data:00569E70                 .word _ZN11ZSysFilePS212SetTitleInfoEPK13STitleInfoPS2  # ZSysFilePS2::SetTitleInfo(STitleInfoPS2 const*)
        .data:00569E74                 .word _ZN11ZSysFilePS210McSaveDataEPvjji  # ZSysFilePS2::McSaveData(void *,uint,uint,int)
        .data:00569E78                 .word _ZN11ZSysFilePS210McLoadDataEPvjji  # ZSysFilePS2::McLoadData(void *,uint,uint,int)
        .data:00569E7C                 .word _ZN11ZSysFilePS213McGetDataSizeEji  # ZSysFilePS2::McGetDataSize(uint,int)
        .data:00569E80                 .word _ZN11ZSysFilePS27McReadyEv  # ZSysFilePS2::McReady(void)
        .data:00569E84                 .word _ZN11ZSysFilePS218CreateSaveGameNameEPcjj  # ZSysFilePS2::CreateSaveGameName(char *,uint,uint)*/
    };
}