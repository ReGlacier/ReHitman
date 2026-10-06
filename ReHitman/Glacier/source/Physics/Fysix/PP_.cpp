#include <Glacier/Physics/Fysix/Fysix.h>


namespace Glacier::Fysix
{
    PP_::~PP_()
    {
        if (particles)
        {
            ZUniMemory::Delete(particles);
        }

        if (groups)
        {
            ZUniMemory::Delete(groups);
        }

        if (constraints)
        {
            ZUniMemory::Delete(constraints);
        }

        if (bodies)
        {
            ZUniMemory::Delete(bodies);
        }
    }
}
