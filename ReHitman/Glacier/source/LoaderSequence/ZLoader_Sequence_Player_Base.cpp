#include <Glacier/LoaderSequence/ZLoader_Sequence_Player_Base.h>

namespace Glacier
{
    // PC 0x473640 (complete object destructor). The deleting destructor at
    // PC 0x473650 frees the object through the compiler-generated scalar delete
    // path; both bodies are empty because the class has no data members.
    // Defining the destructor here anchors the vtable in this translation unit.
    ZLoader_Sequence_Player_Base::~ZLoader_Sequence_Player_Base() = default;
}
