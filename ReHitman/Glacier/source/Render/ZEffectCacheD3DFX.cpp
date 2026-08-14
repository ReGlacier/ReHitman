#include <Glacier/Render/ZEffectCacheD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialResourceD3DFX.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/zstring.h>
#include <cstring>

#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    // ZEffectCacheD3DFX::SFile

    ZEffectCacheD3DFX::SFile::SFile()
        : pszName(nullptr)
        , iFileTime(0)
        , iSize(0)
        , pData(nullptr)
        , lNumIncludes(0)
    {
    }

    // ZEffectCacheD3DFX

    ZEffectCacheD3DFX::ZEffectCacheD3DFX()
    {
        if (ZSysInterface::GetOption("DisableShaderCache", nullptr))
            m_bDisableShaderCache = true;
    }

    ZEffectCacheD3DFX::~ZEffectCacheD3DFX()
    {
        Clear();
    }

    STDMETHODIMP ZEffectCacheD3DFX::Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
    {
        zstring sPath(pFileName);
        if (IncludeType == D3DXINC_LOCAL)
        {
            zstring sShaderPath;
            ZRenderMaterialResourceD3DFX::GetShaderPath(sShaderPath);

            zstring sFullPath;
            sFullPath.format("%s\\%s", sShaderPath.c_str(), pFileName);
            sPath = sFullPath;
        }

        SFile* pFile = FindFile(sPath.c_str());
        if (!pFile)
            return D3DERR_NOTFOUND; // 0x88760866 on PC

        *ppData = pFile->pData;
        *pBytes = static_cast<UINT>(pFile->iSize);
        return S_OK;
    }

    STDMETHODIMP ZEffectCacheD3DFX::Close(LPCVOID pData)
    {
        return S_OK;
    }

    void* ZEffectCacheD3DFX::Load(const char* pszFileName, SFile* pParentFile)
    {
        SFile* pFile = FindFile(pszFileName);
        if (pFile)
        {
            if (pParentFile)
            {
                ZASSERT(pParentFile->lNumIncludes < SFile::MAX_NUM_INCLUDES);
                pParentFile->Includes[pParentFile->lNumIncludes++] = pFile;
            }

            return const_cast<char*>(pFile->pData);
        }

        const int iSize = g_pSysFile->GetSize(pszFileName, false);
        if (iSize == -1)
            return nullptr;

        ZASSERT(m_lNumFiles < MAX_NUM_FILES);
        pFile = &m_Files[m_lNumFiles++];

        if (pParentFile)
        {
            ZASSERT(pParentFile->lNumIncludes < SFile::MAX_NUM_INCLUDES);
            pParentFile->Includes[pParentFile->lNumIncludes++] = pFile;
        }

        pFile->iFileTime = g_pSysFile->GetFileTime64(pszFileName, false);

        char* pszName = static_cast<char*>(ZUniMemory::Allocate(static_cast<int>(std::strlen(pszFileName)) + 1));
        std::strcpy(pszName, pszFileName);
        pFile->pszName = pszName;
        pFile->lNumIncludes = 0;
        pFile->iSize = iSize;

        char* pData = static_cast<char*>(ZUniMemory::Allocate(iSize + 1));
        pData[iSize] = '\0';
        pFile->iSize = g_pSysFile->Load(pszName, pData, iSize, 0, false);
        pFile->pData = pData;
        ZASSERT(iSize == pFile->iSize);

        if (pData)
        {
            // Scan the source for "#include" directives and load each referenced
            // file, resolving it against the directory of the current file.
            const char* pScan = pData;
            while (const char* pInclude = std::strstr(pScan, "#include"))
            {
                char szIncludePath[512];
                std::strcpy(szIncludePath, pszFileName);

                // cut the file name, keep the directory part
                char* pOut = &szIncludePath[std::strlen(szIncludePath) - 1];
                for (; pOut > szIncludePath; --pOut)
                {
                    if (*pOut == '\\' || *pOut == '/')
                        break;
                }
                if (pOut != szIncludePath)
                    ++pOut;

                // skip to the opening quote of the include file name
                const char* pQuote = pInclude;
                for (; *pQuote; ++pQuote)
                {
                    if (*pQuote == '"')
                        break;
                }

                // copy the include file name up to the closing quote
                const char* pChar = pQuote + 1;
                for (char ch = *pChar; ch; ch = *++pChar)
                {
                    if (ch == '"')
                        break;
                    *pOut++ = ch;
                }
                *pOut = '\0';
                pScan = pChar;

                Load(szIncludePath, pFile);
            }
        }

        return const_cast<char*>(pFile->pData);
    }

    void ZEffectCacheD3DFX::Clear()
    {
        for (uint32_t i = 0; i < m_lNumFiles; ++i)
        {
            ZUniMemory::Free(const_cast<char*>(m_Files[i].pszName));
            ZUniMemory::Free(const_cast<char*>(m_Files[i].pData));
        }

        m_lNumFiles = 0;
    }

    bool ZEffectCacheD3DFX::IsUpToDate(const SFile* pFile, uint64_t iFileTime) const
    {
        if (m_bDisableShaderCache)
            return false;

        if (!pFile)
            return false;

        if (pFile->iFileTime > iFileTime)
            return false;

        for (uint32_t i = 0; i < pFile->lNumIncludes; ++i)
        {
            if (!IsUpToDate(pFile->Includes[i], iFileTime))
                return false;
        }

        return true;
    }

    ZEffectCacheD3DFX::SFile* ZEffectCacheD3DFX::FindFile(const char* pszFileName)
    {
        for (uint32_t i = 0; i < m_lNumFiles; ++i)
        {
            if (std::strcmp(m_Files[i].pszName, pszFileName) == 0)
                return &m_Files[i];
        }

        return nullptr;
    }
}
