#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Filesystem/ZSysFile.h>


namespace Glacier
{
    // ZSysFileWintel
    class ZSysFileWintel : public ZSysFile
    {
    public:
        // vtbl
        void Restart() override;
        void PrintStatus() override;
        void* Open(const char* FileName) override;
        void* OpenForAppend(const char* FileName) override;
        void* OpenForRandomAccess(char* FileName) override;
        void* Create(const char* FileName) override;
        bool StartTemp(char* FileName) override;
        void Close(void* Handle) override;
        bool GetFileTime(const char* pFileName, Glacier::_FILETIME* pFileTime, bool NoBig) override;
        uint64_t GetFileTime64(const char* pFileName, bool NoBig) override;
        uint64_t GetCurrentFileTime() override;
        bool FileTimeToSysTime(uint64_t iFileTime, SSystemTime* pSysTime) override;
        int GetSize(const char* pFileName, bool NoBig) override;
        bool Exists(const char* pFileName, bool NoBig) override;
        bool WriteTo(void* WriteTo, void* Ptr, int Size) override;
        int ReadFrom(void* File, void* Ptr, int Size, int lOffset) override;
        int ReadFrom(void* File, void* Ptr, int Size) override;
        int Seek(void* Handle, int iOffset) override;
        void CD(char* DirName) override;
        bool MakeDir(int, int, const char* DirName) override;
        void MakeDir(char* DirName) override;
        void Rename(char* SrcName, char* DstName) override;
        void Copy(char* SrcPath, char* DstPath) override;
        bool Delete(const char* FileName) override;
        int Load(const char* pFileName, void* Ptr, int Size, int Offset, bool NoBig) override;
        void Save(const char* FileName, void* Ptr, int Size, int lWriteSize) override;
        void Append(const char* FileName, void* Ptr, int Size) override;
        int LoadLib(const char* pszLib) override;
        bool FreeLib(int hLib) override;
        void GetDllExports(const char* pLibraryFileName, CHUNK* pClassInfoChunk, CHUNK* pProcNamesChunk) override;
        ~ZSysFileWintel() override;
        bool SaveGame(void* pData, uint32_t iSize, uint32_t iSaveGameNumber, uint32_t iSaveGamePart) override;
        bool LoadGame(void* pData, uint32_t iSaveGameNumber, uint32_t iSaveGamePart) override;
        int GameSize(uint32_t iSaveGameNumber, uint32_t iSaveGamePart) override;
        void RemoveSaveGame(uint32_t iSaveGameNumber) override;
        uint32_t MaxNumPorts() override { return 1u; }
        void EnumSaveGames(uint32_t, uint32_t, STRREFTAB* ) override;
        void GetFilesInDir(STRREFTAB* paFiles, const char* pDirPath) override;
        int CreateSaveGame(uint32_t, uint32_t, const char* , const void* , uint32_t, uint32_t* ) override;
        int LoadSaveGame(uint32_t, uint32_t, const char* , void* , uint32_t) override;
        int SaveGameSize(uint32_t, uint32_t, const char* , uint32_t* ) override;
        int DeleteSaveGame(uint32_t, uint32_t, const char* ) override;
        MYSTR ConvertFileNameFromZipToHD(const char* ) override;
        int GetStatus(int, int, int* ) override { return 0; }
        uint32_t GetNumSlots(int) override { return 0u; }
        bool IsValid(uint32_t a, uint32_t b) override { return ((static_cast<uint64_t>(a) << 32) | static_cast<uint64_t>(b)) == 0u; }
        // new methods in PC
        virtual void EnumSaveGames(STRREFTAB* pFound, const char* pDirPath);
        virtual int32_t GetFilePos(void* vfile, int sectPos, uint32_t NamePos);
        virtual void LoadDllExports(const char* pLibraryFileName, CHUNK* pClassInfoChunk, CHUNK* pProcNamesChunk);
        virtual uint64_t GetFreeDiskSpace(const char* szDirectory);
        virtual void CreateSaveGameName(char* psOutSaveName, uint32_t iSaveGameNumber, uint32_t iSaveGamePart);

        // methods
        ZSysFileWintel();

        // members
        LINKREFTAB* m_prtLibraries;
        int m_field110{0}; // +0x110 - UNUSED
        int m_field114{0}; // +0x114 - UNUSED
    };
    RE_VERIFY_SIZE(ZSysFileWintel, 0x118); // Verified in PC
    RE_VERIFY_OFFSET(ZSysFileWintel, m_prtLibraries, 0x10C); // Verified in PC constructor
}