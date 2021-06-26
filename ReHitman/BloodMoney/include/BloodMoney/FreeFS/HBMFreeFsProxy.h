#pragma once

#include <string>

namespace Hitman::BloodMoney::FreeFS {
    class HBMFreeFsProxy {
    public:
        char pad_0000[16];			//0x0000
        char* m_missionZipFilePath; //0x0010
        char pad_0014[100];			//0x0014
        char* m_zipFilePath;		//0x0078

        static std::string findFileInFolderRecursively(const std::string& folder, const std::string& file);

        int readFileProvider(const char* name, void* dest, int fileSize, int unk1);
        int getFileSizeProvider(const char* name);
    };
}