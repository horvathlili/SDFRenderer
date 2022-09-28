#include "../Utils/sdf.hlsl"

RWTexture3D<float4> tex3D_uav;
RWStructuredBuffer<float> x0;

cbuffer csCb
{
    int sdf;
    float3 boundingBox;
    int res;
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

    //xi meghatározása
    float3 xi = -boundingBox / 2.0 + (threadId.xyz + 0.5) * (boundingBox / res);

    float f[27];

    //kisebb grid xi körül

    float3 x;

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            for (int k = -1; k <= 1; k++)
            {
                x = xi + float3(i, j, k) * (boundingBox / res) * 0.4;

                f[(i + 1) * 9 + (j + 1) * 3 + (k + 1)] = getDist(x);
               
            }
        }

    }


    float4 ai;
    
    for (int l = 0; l < 4; l++)
    {
        float aa = 0;
        for (int j = 0; j < 27; j++)
        {
            aa += x0[l * 27 + j] * f[j];
        }
       if (l == 3)
        {
            aa -= (ai[0] * xi.x + ai[1] * xi.y + ai[2] * xi.z);
        }
        ai[l] = aa;
    }
   
    float n = sqrt(ai.r * ai.r + ai.g * ai.g + ai.b * ai.b);
    float4 a = { ai.r / n, ai.g / n, ai.b / n, ai.a/ n };
    
    tex3D_uav[threadId.xyz] = a;
}
