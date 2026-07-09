#pragma once

#include <Glacier/Component/ZComponent.h>


namespace Glacier
{
    struct ZComponentDefaultAllocator {};
    struct ZGlobalComponentAllocator {};

    template <size_t ComponentGroup, typename AllocatorTag>
    struct ZComponentDescription : public ZComponentBase
    {
    };

    template <typename T> using ZComponentDefaultAllocatorDescription 
        = ZComponent<T, ZComponentDescription<0, ZComponentDefaultAllocator>>;
    template <typename T> using ZGlobalComponentAllocatorDescription 
        = ZComponent<T, ZComponentDescription<1, ZGlobalComponentAllocator>>;
}