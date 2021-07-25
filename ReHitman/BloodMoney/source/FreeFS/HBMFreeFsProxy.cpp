#include <BloodMoney/FreeFS/HBMFreeFsProxy.h>
#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/FsZip_t.h>

#include <filesystem>
#include <spdlog/spdlog.h>
#include <BloodMoney/Game/Globals.h>

namespace Hitman::BloodMoney::FreeFS {
    namespace Consts {
        static constexpr std::intptr_t FsZip_Read_OriginalPtr = 0x0042C470;
        static constexpr std::intptr_t FsZip_GetFileSize_OriginalPtr = 0x0042C2D0;
    }

    std::string HBMFreeFsProxy::findFileInFolderRecursively(const std::string& folder, const std::string& file) try
    {
        namespace fs = std::filesystem;

        try {
            for (const auto& entry : fs::directory_iterator(folder)) {
                const auto filenameStr = entry.path().filename().string();
                if (entry.is_directory()) {
                    auto result = findFileInFolderRecursively(entry.path().string(), file);
                    if (!result.empty())
                        return result;
                }
                else if (entry.is_regular_file()) {
                    if (filenameStr == file)
                        return entry.path().string();
                }
            }
        }
        catch (...) {
            return "";
        }

        return "";
    }
    catch (const std::exception & ex)
    {
        spdlog::error("FreeFS exception: {}", ex.what());
        assert(false && "See log for details");
        return "";
    }

    int HBMFreeFsProxy::readFileProvider(const char* name, void* dest, int fileSize, int unk1) {
        namespace fs = std::filesystem;

        auto sys = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);

        // --- detect scene ---
        std::string fileName = name;
        std::string zipPackageFileName = fs::path(this->m_missionZipFilePath).stem().string();
        std::string sceneName = fs::path(sys->m_currentScene).stem().string();

        if (fileName[0] == '*')
        {
            fileName = findFileInFolderRecursively(
                    fmt::format("UnpackedScenes\\{}\\{}", sceneName, zipPackageFileName),
                    fileName.substr(1, fileName.length())
            );
        }

        std::string path = fmt::format(R"(UnpackedScenes\{}\{}\{})", sceneName, zipPackageFileName, fileName);

        FILE* fp = fopen(path.c_str(), "rb");
        if (fp)
        {
            if (fileSize == 0)
            {
                // The glacier doesn't know the actual size of file and pass zero to us
                // We must recognize how much bytes will be used by file in memory space.
                fseek(fp, 0L, SEEK_END);	//move to end
                fileSize = ftell(fp); //save the endpoint
                rewind(fp); //move to start of file
            }

            const int readyBytes = fread(dest, 1, fileSize, fp);
            fclose(fp);
            spdlog::info("FsZip::readContents| Read file from fs {} (got bytes {})", path, readyBytes);
            return readyBytes;
        }

        spdlog::warn("FsZip::readContents| Read file from ZIP {}", path);

        // original code
        typedef int(__thiscall* FsZip_read_t)(Glacier::FsZip_t*, const char*, void*, int, int);
        auto original = (FsZip_read_t)Consts::FsZip_Read_OriginalPtr;
        int result = original(reinterpret_cast<Glacier::FsZip_t*>(this), name, dest, fileSize, unk1);
        return result;
    }

    int HBMFreeFsProxy::getFileSizeProvider(const char* name) {
        namespace fs = std::filesystem;

        auto sys = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);

        // --- detect scene ---
        std::string fileName = name;
        std::string zipPackageFileName = fs::path(this->m_missionZipFilePath).stem().string();
        std::string sceneName = fs::path(sys->m_currentScene).stem().string();

        if (fileName[0] == '*')
        {
            fileName = findFileInFolderRecursively(
                    fmt::format("UnpackedScenes\\{}\\{}", sceneName, zipPackageFileName),
                    fileName.substr(1, fileName.length())
            );
        }

        std::string path = fmt::format(R"(UnpackedScenes\{}\{}\{})", sceneName, zipPackageFileName, fileName);

        FILE* fp = fopen(path.c_str(), "rb");
        if (fp)
        {
            int fileSize = 0;
            fseek(fp, 0L, SEEK_END);	//move to end
            fileSize = ftell(fp); //save the endpoint
            fclose(fp);

            spdlog::info("FsZip::getFileSize| Got file size {} from fs {}", fileSize, path);
            return fileSize;
        }

        // original code
        typedef int(__thiscall* FsZip_getFileSize_t)(Glacier::FsZip_t*, const char*);
        auto original = (FsZip_getFileSize_t)Consts::FsZip_GetFileSize_OriginalPtr;
        int result = original(reinterpret_cast<Glacier::FsZip_t*>(this), name);

        spdlog::warn("FsZip::getFileSize| Got file size {} from ZIP {}", result, path);
        return result;
    }
}