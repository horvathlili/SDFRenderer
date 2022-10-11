#include "../Utils/sdf.hlsl"

RWTexture3D<float4> tex3D_uav;

cbuffer csCb
{
    int sdf;
    int res;
    float boundingBox;
};

float getDist(float3 pos)
{
    return map(pos, sdf);
}

[numthreads(8, 8, 8)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    if (any(threadId.xyz >= float3(res)))
        return;
    float3 t = float3(-1) + (threadId.xyz + 0.5) * 2.0 / (float3) res;
    float4 val = float4(getDist(t), getNormal(t, sdf));
    tex3D_uav[threadId.xyz] = val;
}
