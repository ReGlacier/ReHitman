#include <Glacier/ScriptEngine/SF.h>
#include <Glacier/ScriptEngine/Globals.h>
#include <Glacier/ZSysInterface.h>
#include <cstring>
#include <cstdarg>


namespace Glacier
{
    bool SF_CheckTimeout()
    {
        return g_pSysInterface->TimeStampCounter(__FILE__, __LINE__) - g_lCurrentScriptMaxTime >= 5001;
    }

    void SF_DebugPrint(const char* format, ...)
    {
        char buffer[1024] { 0 };

        va_list args;
        va_start(args, format);
        vsprintf(buffer, format, args);
        va_end(args);

        printf("ZScriptC : %s", buffer);
    }

    void SF_GetZDefine(const char* pName, void* pData, uint32_t lSize)
    {
        // Yep, from PS2 (on PC do nothing)
        memset(pData, 0x0, lSize);
    }

    void SF_Input(void* pData, uint32_t lSize)
    {
        ZASSERT(g_pZScriptCDataBlock != nullptr);

        memcpy(pData, g_pZScriptCDataBlock, lSize);
        g_pZScriptCDataBlock += lSize;
    }

    void SF_Memcpy(void* dest, void* source, uint32_t lSize)
    {
        ZASSERT(dest && source);

        memcpy(dest, source, lSize);
    }

    void SF_Memset(void* dest, uint8_t data, uint32_t lSize)
    {
        ZASSERT(dest);

        memset(dest, data, lSize);
    }

    void SF_Pack(void* pData, uint32_t lSize)
    {
        memset(pData, 0, lSize);
    }

    void SF_Unpack(void* pData, uint32_t lSize)
    {
        memset(pData, 0, lSize);
    }
}