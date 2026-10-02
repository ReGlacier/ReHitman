#include <Glacier/Data/ZLoadGameInfoPC.h>

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

    // PC 0x675240.
    IInputStream* ZLoadGameInfoPC::CreateStream(const char* pszFilename)
    {
        // Allocates a 12-byte ZFileInputStreamWintel that reads pszFilename.
        // TODO: Finish me after ZFileInputStreamWintel reversed.
        //   void* pMem = ZSysMem::allocate(12, "hitman3\\gui\\savepc.cpp", 354);
        //   if (pMem) return ZFileInputStreamWintel::ZFileInputStreamWintel(pMem, pszFilename);
        //   return 0;
        (void)pszFilename;
        return nullptr;
    }
}
