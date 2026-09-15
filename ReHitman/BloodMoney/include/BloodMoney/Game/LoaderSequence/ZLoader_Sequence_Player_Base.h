#pragma once

#include <Glacier/ReGlacier.h>


namespace Hitman::BloodMoney
{
    class ZLoader_Sequence_Player_Base
    {
    public:
        virtual ~ZLoader_Sequence_Player_Base();
        virtual void Start();
        virtual void Finish();
        virtual void Progress(float fValue);
        virtual void Disable_Render();
        virtual void Enable_Render();
    };
}