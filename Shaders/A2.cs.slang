#include "../Utils/sdf.hlsl"

RWTexture3D<float4> tex3D_uav1;
RWTexture3D<float4> tex3D_uav2;
RWTexture3D<float4> tex3D_uav3;
RWStructuredBuffer<float> x0;

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

    float3 delta = boundingBox / float3(res);
    
    float3 xi = (threadId.xyz + 0.5) * delta - boundingBox / 2.0;


    float f[27];
    
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            for (int k = -1; k <= 1; k++)
            {
                float3 x = xi + float3(i, j, k) * 0.6f * delta;

                f[(i + 1) * 9 + (j + 1) * 3 + (k + 1)] = getDist(x);
            }
        }
    }

    float ai[10];

    for (int l = 0; l < 10; l++)
    {
        float aa = 0;
        for (int j = 0; j < 27; j++)
        {
            aa += x0[l * 27 + j] * f[j];
        }
        ai[l] = aa;
    }

    ai[9] -= (ai[0] * xi.x + ai[1] * xi.y + ai[2] * xi.z
            + ai[3] * xi.x * xi.x + ai[4] * xi.y * xi.y  + ai[5] * xi.z * xi.z
            + ai[6] * xi.x * xi.y + ai[7] * xi.x * xi.z + ai[8] * xi.y * xi.z);
    //float n = sqrt(ai.r * ai.r + ai.g * ai.g + ai.b * ai.b);
    
    tex3D_uav1[threadId.xyz] = float4(ai[0],ai[1],ai[2],ai[3]);
    tex3D_uav2[threadId.xyz] = float4(ai[4], ai[5], ai[6], ai[7]);
    tex3D_uav3[threadId.xyz] = float4(ai[8], ai[9], 0, 0);

}
