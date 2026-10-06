#pragma once


namespace Glacier::Locomotion
{
    class ZState;

    const char* GetProgramName(int lId);

    // Locomotion program table accessor (PC off_7FC500): the singleton state
    // objects indexed by the PF4 program id.
    ZState* GetProgram(int lProgram);
}
