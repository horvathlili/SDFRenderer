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

RWStructuredBuffer<float3> points;

cbuffer psCb
{
    float4 eye;
    float4 center;
    float4 up;
    float ar;
    int n;
};



PsOut main(psIn input) : SV_TARGET
{
    PsOut o;
    o.col = float4(0, 0, 0, 1);
    Ray r = getRay(eye.xyz, center.xyz, up.xyz, ar, input.pixelpos.x, input.pixelpos.y);

     
    for (int i = 0; i < n; i++)
    {
        float3 d = normalize(points[i] - r.p);
        if (clamp(dot(d, r.v), 0, 1) > 0.9999)
        {
            o.col = float4(1, 0, 0, 1);
        }
    }
    
    
       
    // calc fragment depth
    //float4 depth_vec = mul(float4(t * r.v + r.p, 1), viewproj);
    o.depth = 1;

    return o;
}
