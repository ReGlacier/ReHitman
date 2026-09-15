#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/D3D9.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @brief Source-level cache for D3DX effect (.fx) files used by the shader pipeline.
     *
     * ZEffectCacheD3DFX stores the contents of shader source files together with their
     * last-write times and #include dependencies. It serves two purposes:
     *  - it implements ID3DXInclude, so the D3DX effect compiler resolves #include
     *    directives against the cached file contents instead of reading from disk;
     *  - it tracks file times of sources and their includes, which allows
     *    ZRenderMaterialResourceD3DFX::CreateEffect to validate the compiled-effect
     *    cache and skip recompilation when the sources have not changed.
     *
     * The cache holds at most MAX_NUM_FILES files and is flushed as a whole via Clear().
     * Shader caching can be forced off with the "DisableShaderCache" configuration
     * option (see m_bDisableShaderCache).
     */
    class ZEffectCacheD3DFX : public ID3DXInclude
    {
    public:
        // constants
        /** @brief Maximum number of source files the cache can hold. */
        static constexpr uint32_t MAX_NUM_FILES = 0x40; // Verified with PC (bound check in ZEffectCacheD3DFX::Load)

        // types
        /**
         * @brief Cached shader source file record.
         *
         * Owns the file name and the file contents (allocated via ZUniMemory and
         * released by ZEffectCacheD3DFX::Clear()) and lists the records of the
         * files it includes (non-owning pointers into ZEffectCacheD3DFX::m_Files).
         */
        struct SFile
        {
            // constants
            /** @brief Maximum number of include records a single file can have. */
            static constexpr uint32_t MAX_NUM_INCLUDES = 0x10; // Verified with PC (bound check in ZEffectCacheD3DFX::Load)

            // members
            const char* pszName;                     // 0x00
            uint64_t iFileTime;                      // 0x08
            int iSize;                               // 0x10
            const char* pData;                       // 0x14
            uint32_t lNumIncludes;                   // 0x18
            const SFile* Includes[MAX_NUM_INCLUDES]; // 0x1C

            // methods
            /** @brief Constructs an empty record with no name, contents or includes. */
            SFile();
        };

        // vtbl
        /**
         * @brief ID3DXInclude callback that resolves an include file to cached data.
         *
         * For D3DXINC_LOCAL includes the file name is resolved against the shaders
         * directory (ZRenderMaterialResourceD3DFX::GetShaderPath); other include
         * types are looked up as-is. Only files already present in the cache are
         * accepted - Open() never reads from disk, Load() does that upfront.
         *
         * @param IncludeType D3DX include type (D3DXINC_LOCAL / D3DXINC_SYSTEM).
         * @param pFileName Include file name as written in the shader source.
         * @param pParentData Parent data pointer (unused).
         * @param ppData Receives a pointer to the cached file contents (owned by the cache).
         * @param pBytes Receives the cached file size in bytes.
         * @return S_OK when the file is found in the cache, D3DERR_NOTFOUND otherwise.
         */
        STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override;

        /**
         * @brief ID3DXInclude callback that releases data returned by Open().
         *
         * This is a no-op: the data is owned by the cache and released by Clear().
         *
         * @param pData Data pointer previously returned by Open() (unused).
         * @return Always S_OK.
         */
        STDMETHOD(Close)(LPCVOID pData) override;

        // methods
        /**
         * @brief Constructs an empty cache and applies the "DisableShaderCache" option.
         */
        ZEffectCacheD3DFX();

        /**
         * @brief Releases all cached files (see Clear()).
         */
        ~ZEffectCacheD3DFX();

        /**
         * @brief Loads a shader source file into the cache and returns its contents.
         *
         * When the file is already cached, the existing record is reused. Otherwise
         * the file is read from disk (via g_pSysFile) together with its last-write
         * time, and the source is scanned for "#include" directives; every referenced
         * file is loaded recursively, resolving relative names against the directory
         * of the including file.
         *
         * @param pszFileName Path of the shader source file.
         * @param pParentFile File that includes this file, or nullptr for a top-level
         *                    file. The file record is appended to the parent's
         *                    Includes list.
         * @return Pointer to the cached file contents, or nullptr when the file
         *         does not exist.
         */
        void* Load(const char* pszFileName, SFile* pParentFile);

        /**
         * @brief Frees the name and contents of every cached file and empties the cache.
         */
        void Clear();

        /**
         * @brief Checks whether a cached source file predates the given time stamp.
         *
         * Used to validate the compiled-effect cache: returns true when the file and
         * all of its includes (recursively) are not newer than @p iFileTime. Always
         * returns false when the shader cache is disabled (m_bDisableShaderCache).
         *
         * @param pFile Cached file record to check, or nullptr.
         * @param iFileTime Reference time stamp (e.g. the compiled effect file time).
         * @return true when the file tree is up to date with respect to @p iFileTime.
         */
        bool IsUpToDate(const SFile* pFile, uint64_t iFileTime) const;

        /**
         * @brief Finds a cached file record by file name.
         *
         * @param pszFileName File name to look up (exact string match).
         * @return Pointer to the cached record, or nullptr when the file is not cached.
         */
        SFile* FindFile(const char* pszFileName);

        // members
        /** @brief When set, cached effects are always treated as outdated ("DisableShaderCache" option). */
        bool m_bDisableShaderCache { false };   // 0x04
        /** @brief Number of valid records in m_Files. */
        uint32_t m_lNumFiles { 0u };            // 0x08
        /** @brief Fixed storage of cached file records. */
        SFile m_Files[MAX_NUM_FILES];           // 0x10
    };
    RE_VERIFY_SIZE(ZEffectCacheD3DFX::SFile, 0x60);
    RE_VERIFY_SIZE(ZEffectCacheD3DFX, 0x1810);
}
