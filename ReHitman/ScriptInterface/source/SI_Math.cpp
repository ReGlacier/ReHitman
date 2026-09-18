#include <SI/SI_Math.h>
#include <numbers>


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
        vmuls(result, source, scalar);
    }

    void Math__Vaddscalar(v3& result, v3 first, v3 second, float scalar)
    {
        vaddscalar(result, first, second, scalar);
    }

    float Math__Vdist(v3 first, v3 second)
    {
        return vdist(first, second);
    }

    void Math__Vsub(v3& result, v3 first, v3 second)
    {
        vsub(result, first, second);
    }

    void Math__Vadd(v3& result, v3 first, v3 second)
    {
        vadd(result, first, second);
    }

    void Math__Vneg(v3& result)
    {
        vneg(result);
    }

    void Math__Vreset(v3& result)
    {
        vreset(result);
    }

    float Math__Vdot(v3 first, v3 second)
    {
        return vdot(first, second);
    }

    float Math__Vlen(v3 value)
    {
        return vlen(value);
    }

    void Math__Vnorm(v3& value)
    {
        vnorm(value);
    }

    void Math__Vpul(v3& result, v3 first, v3 second, float factor)
    {
        result.x = (second.x - first.x) * factor + first.x;
        result.y = (second.y - first.y) * factor + first.y;
        result.z = (second.z - first.z) * factor + first.z;
    }

    float Math__Sqrtf(float value)
    {
        return sqrt(value);
    }

    void Math__Vrot(v3& result, v3 rotation)
    {
        constexpr float DEG2RAD = std::numbers::pi_v<float> / 180.0f;

        vscalar(rotation, DEG2RAD);
        vrot(result, rotation);
    }
}
