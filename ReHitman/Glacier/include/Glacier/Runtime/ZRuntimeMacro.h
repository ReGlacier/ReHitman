#pragma once

#include <Glacier/ZUniMemory.h>
#include <cstdint>


#define DECLARE_ID_AND_MASK(cls) \
    STATIC_CLASS_VAR(cls, uint32_t, m_Id); \
    STATIC_CLASS_VAR(cls, uint32_t, m_Mask);

#define DECLARE_ID_AND_MASK_IMPL(cls, id_addr, mask_addr) \
    STATIC_CLASS_VAR_IMPL(cls, uint32_t, m_Id, id_addr, 0u); \
    STATIC_CLASS_VAR_IMPL(cls, uint32_t, m_Mask, mask_addr, 0u);
