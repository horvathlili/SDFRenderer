#include "../Utils/Ray.hlsl"
#include "../Utils/sdf.hlsl"

struct psIn
{
    float4 pos : SV_POSITION;
    float3 col : COLOR;
    float3 pixelpos : POSITION;
};

struct PsOut
{
    float4 col : SV_TARGET;
    float depth : SV_Depth;
};

RWStructuredBuffer<float4> points;

cbuffer psCb
{
    float4 eye;
    float4 center;
    float4 up;
    float ar;
    int n;
    float r;
    float R;
    float3 c;
    float3 d;
    float4 viewproj;
};



PsOut main(psIn input) : SV_TARGET
{
    PsOut o;
    o.col = float4(0, 0, 0, 1);
    Ray ray = getRay(eye.xyz, center.xyz, up.xyz, ar, input.pixelpos.x, input.pixelpos.y);

    float t = 0.0f;
    int i;
    int n = 100;
    float3 pos = ray.p + t * ray.v;
    
    for (i = 0; i < n; i++)
    {
        pos = ray.p + t * ray.v;
        float h = sdTorus2(pos, r, R, c, float3(d.x, d.z, d.y));
        if (h < 0.0001)
            break;
        t += h;
    }
    

    if (i < n)
        o.col = float4(1, 1, 1, 1);
    // calc fragment depth
    float4 depth_vec = mul(float4(t * ray.v + ray.p, 1), viewproj);
    o.depth = depth_vec.z / depth_vec.w;
    o.depth = 1;

    return o;
}
