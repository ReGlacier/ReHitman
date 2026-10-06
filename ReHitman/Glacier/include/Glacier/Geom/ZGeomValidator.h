#pragma once

#include <concepts>
#include <cstdint>
#include <utility>


namespace Glacier
{
    template <typename T>
    concept HasGeomIDAndMask = requires {
        T::m_Id;
        T::m_Mask;
    } && std::is_assignable_v<decltype(T::m_Id)&, uint32_t> && std::is_assignable_v<decltype(T::m_Mask)&, uint32_t>;

    template <HasGeomIDAndMask TGeom>
    struct ZGeomValidator
    {
        static void SetTypeIDAndMask(uint32_t lId, uint32_t lMask) 
        {
            TGeom::m_Id = lId;
            TGeom::m_Mask = lMask;
        }
    };

}