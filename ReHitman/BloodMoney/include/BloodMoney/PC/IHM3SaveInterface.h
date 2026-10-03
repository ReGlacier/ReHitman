#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Data/ILoadCallBack.h>
#include <Glacier/ZSTL/STLport.h>
#include <Glacier/ZSTL/zstring.h>


namespace Hitman
{
    struct SProfileInfo
    {
        // methods
        SProfileInfo();
        SProfileInfo(const SProfileInfo& copy);
        bool operator==(const SProfileInfo& rhs) const;

        // members
        Glacier::zstring sProfileName;
        int iProfileId;
        bool bCorrupt;
        RE_ADD_PADDING(3);
    };

    struct SGameInfo
    {
        // methods
        SGameInfo();
        SGameInfo(const SGameInfo& copy);
        bool operator==(const SGameInfo& rhs) const;

        // members
        int iLevelNum;
        int iSaveSlot;
        uint64_t iFileTime;
        bool bEmpty;
    };

    using ENUMERATED_PROFILES = stlp::vector<SProfileInfo>;

    class IHM3SaveInterface : public Glacier::ILoadCallBack
    {
    public:
        // vtbl
        virtual bool SaveDeviceChangeSinceLastSave() = 0;
        virtual void ResetSaveDeviceChangeSinceLastSave() = 0;
        virtual bool CheckSaveDevice() = 0;
        virtual int DoSaveDeviceCheck() = 0;
        virtual int CreateNewProfile(bool) = 0;
        virtual int LoadProfile(bool) = 0;
        virtual int SaveProfile(bool) = 0;
        virtual int DeleteProfile(bool) = 0;
        virtual int EnumerateProfiles(bool) = 0;
        virtual int CreateNewSaveGame(bool) = 0;
        virtual int SaveGame(bool) = 0;
        virtual int LoadGame(bool) = 0;
        virtual int EnumerateGames(bool) = 0;
        virtual int Format(bool) = 0;
        virtual int MaxNumberOfSaveGames() = 0;
        virtual int BootCheck() = 0;
        virtual int GetSaveGameSize() = 0;
        virtual int GetMaxNumberOfProfiles() = 0;
        virtual int GetMissingSpace() = 0;
        virtual int GetStatus() = 0;
        virtual void SetSaveGameSlot(int iSlotIdx) = 0;
        virtual int GetSaveGameSlot() = 0;
        virtual const ENUMERATED_PROFILES* GetEnumeratedProfiles() = 0;
        virtual const SGameInfo* GetGameInfo(int iGameIdx) = 0;
        virtual int GetNumOfGameInfo() = 0;
        virtual const SProfileInfo* GetProfileInfo(const char* psProfileName) = 0;
    };
}
