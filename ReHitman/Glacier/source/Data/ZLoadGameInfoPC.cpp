#include <Glacier/Data/ZLoadGameInfoPC.h>
#include <Glacier/Serializer/ZFileInputStreamWintel.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    // PC 0x6752C0. The base constructor registers m_Instance and the compiler
    // installs the PC vftable; the caller then supplies the file names via
    // SetFilenames (see the save-game creation path).
    ZLoadGameInfoPC::ZLoadGameInfoPC()
    {
    }

    // PC 0x675330 / 0x675350.
    ZLoadGameInfoPC::~ZLoadGameInfoPC()
    {
    }

    // PC 0x675240. Allocates a 12-byte ZFileInputStreamWintel (the PC calls
    // ZSysMem::allocate(0xC, "hitman3\\gui\\savepc.cpp", 354) first) that reads
    // pszFilename; returns nullptr when the allocation fails.
    IInputStream* ZLoadGameInfoPC::CreateStream(const char* pszFilename)
    {
        return ZUniMemory::New<ZFileInputStreamWintel>(pszFilename);
    }
}
