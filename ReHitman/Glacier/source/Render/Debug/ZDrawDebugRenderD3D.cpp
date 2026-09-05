#include <Glacier/Render/Debug/ZDrawDebugRenderD3D.h>


namespace Glacier
{
    namespace 
    {
        static constexpr const char* g_strPixelShaderProgram__3PBDB = R"(
sampler s : register(s0);

struct PS_IN
{
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PS_IN In) : COLOR0
{
    return In.Color * tex2D(s, In.TexCoord);
}
        )";

        static constexpr const char* g_strVertexShaderProgram__3PBDB = R"(
float4x4 matWVP : register(c0);

struct VS_IN
{
    float4 ObjPos   : POSITION;
    float4 Color    : COLOR0;
    float3 TexCoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 ProjPos  : POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

VS_OUT main(VS_IN In)
{
    VS_OUT Out;
    
    Out.ProjPos = mul(matWVP, In.ObjPos);
    Out.Color   = In.Color;    
    Out.TexCoord = In.TexCoord.xy;
    
    return Out;
}
)";
    }
    // ZDrawDebugRenderD3D
}