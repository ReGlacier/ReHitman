#pragma once

#include <Glacier/Glacier.h>

namespace Glacier
{
    class ResourceCollection {
    public:
        uint32_t m_field4;

        virtual void* GetResourceText(char const*); //#0000 at 00550F58
        virtual void* GetResourceText(char const*,char const*); //#0001 at 0014CD94 org ResourceCollection::GetResourceText(char const*,char const*)
        virtual void* GetElementText(char const*,int); //#0002 at 0014CE34 org ResourceCollection::GetElementText(char const*,int)
    }; //End of ResourceCollection from 00550F58
}