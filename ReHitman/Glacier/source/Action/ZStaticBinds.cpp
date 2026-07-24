#include <Glacier/Action/ActionInterface.h>


namespace Glacier
{
    Action::ZStaticBinds* Action::ZStaticBinds::GetFirst()
    {
        return Action::ZStaticBinds::pFirst;
    }
            
    Action::ZStaticBinds::ZStaticBinds(const char* psBinds)
    {
        szBinds = psBinds;
        pNext = Action::ZStaticBinds::pFirst;
        Action::ZStaticBinds::pFirst = this;
    }

    Action::ZStaticBinds* Action::ZStaticBinds::GetNext() const
    {
        return pNext;
    }

    const char* Action::ZStaticBinds::GetBinds() const
    {
        return szBinds;
    }

    STATIC_CLASS_VAR_IMPL(Action::ZStaticBinds, Action::ZStaticBinds*, pFirst, 0x008ACAC8, nullptr);
}