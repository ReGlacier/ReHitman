#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Filesystem/IOLegacyFilesystem_t.h>


namespace Glacier
{
    enum IOFSAccess_t : int32_t
    {
        IOFS_NOACCESS = 0x0,
        IOFS_CREATE = 0x1,
        IOFS_READONLY = 0x2,
        IOFS_READ = 0x3,
        IOFS_WRITE = 0x4,
        IOFS_PRECACHE = 0x5,
        IOFS_ASYNC = 0x6,
    };

    struct IOFSHandle_t
    {
        IOFSAccess_t fileAccess;
    };

    struct IOFilesystem_t : public IOLegacyFilesystem_t
    {
        // vtbl (overrides?)
        // TODO: Finish me

        // -> IOFilesystem_t
        ~IOFilesystem_t() override = default;
        virtual bool initFS(const char *, IOFSAccess_t) = 0;
        virtual void unloadFS() = 0;
        virtual IOFSHandle_t* open(const char *, IOFSAccess_t) = 0;
        virtual int read(IOFSHandle_t *, void *, unsigned int) = 0;
        virtual int write(IOFSHandle_t *, void *, unsigned int) = 0;
        virtual void close(IOFSHandle_t *) = 0;
        virtual bool eof(IOFSHandle_t *) = 0;
        virtual bool Compare(const char *, const char *) = 0;
        virtual bool InvalidateFile(const char *) = 0;

        // methods
        // members
        IOFSAccess_t fsAccess; // +0xC
        char* fsName; // +0x10
    };
    RE_VERIFY_SIZE(IOFilesystem_t, 0x14);
}