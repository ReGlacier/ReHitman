#include <G1ConfigurationService.h>

namespace Glacier
{
    G1ConfigurationService& G1ConfigurationService::GetInstance()
    {
        static G1ConfigurationService instance;
        return instance;
    }
}