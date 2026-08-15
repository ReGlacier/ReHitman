#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9shader.h>
#include <d3d9types.h>


#define D3D_SAFE_RELEASE(x) \
    if ((x) && !((x)->Release())) { \
        (x) = nullptr; \
    }
