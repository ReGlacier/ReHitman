#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    void Math__Vset(v3& result, float x, float y, float z);
    void Math__Vcpy(v3& result, v3 source);
    void Math__Vscalar(v3& result, v3 source, float scalar);
    void Math__Vaddscalar(v3& result, v3 first, v3 second, float scalar);
    float Math__Vdist(v3 first, v3 second);
    void Math__Vsub(v3& result, v3 first, v3 second);
    void Math__Vadd(v3& result, v3 first, v3 second);
    void Math__Vneg(v3& result);
    void Math__Vreset(v3& result);
    float Math__Vdot(v3 first, v3 second);
    float Math__Vlen(v3 value);
    void Math__Vnorm(v3& value);
    void Math__Vpul(v3& result, v3 first, v3 second, float factor);
    float Math__Sqrtf(float value);
    void Math__Vrot(v3& result, v3 rotation);
}
