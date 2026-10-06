#include <Glacier/Filesystem/ZIP.h>
#include <cstring>


namespace Glacier
{
    IOZip_EndOfCentralDir_t::IOZip_EndOfCentralDir_t() = default;

    void IOZip_EndOfCentralDir_t::reset()
    {
        memset(this, 0, sizeof(IOZip_EndOfCentralDir_t));
    }
}
