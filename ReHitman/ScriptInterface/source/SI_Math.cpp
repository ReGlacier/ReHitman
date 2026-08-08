#include <SI/SI_Math.h>

namespace Glacier
{
    void Math__Vset(v3& result, float x, float y, float z)
    {
        result = { x, y, z };
    }

    void Math__Vcpy(v3& result, v3 source)
    {
        result = source;
    }

    void Math__Vscalar(v3& result, v3 source, float scalar)
    {
        // TODO: Finish me
    }

    void Math__Vaddscalar(v3& result, v3 first, v3 second, float scalar)
    {
        // TODO: Finish me
    }

    float Math__Vdist(v3 first, v3 second)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Math__Vsub(v3& result, v3 first, v3 second)
    {
        // TODO: Finish me
    }

    void Math__Vadd(v3& result, v3 first, v3 second)
    {
        // TODO: Finish me
    }

    void Math__Vneg(v3& result)
    {
        result.x = -result.x;
        result.y = -result.y;
        result.z = -result.z;
    }

    void Math__Vreset(v3& result)
    {
        result = { 0.f, 0.f, 0.f };
    }

    float Math__Vdot(v3 first, v3 second)
    {
        // TODO: Finish me
        return 0.0f;
    }

    float Math__Vlen(v3 value)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Math__Vnorm(v3& value)
    {
        // TODO: Finish me
    }

    void Math__Vpul(v3& result, v3 first, v3 second, float factor)
    {
        // TODO: Finish me
    }

    float Math__Sqrtf(float value)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Math__Vrot(v3& result, v3 rotation)
    {
        // TODO: Finish me
    }
}
