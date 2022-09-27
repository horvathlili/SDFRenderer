#include "../Utils/sdf.hlsl"

RWTexture3D<float4> tex3D_uav;

cbuffer csCb
{
    int sdf;
    float3 boundingBox;
    int res;
    float x0[4][27];
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

    float m[27][4];
    float f[27];

    //kisebb grid xi körül

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            for (int k = -1; k <= 1; k++)
            {
                float3 x = xi + float3(i, j, k) / 3 * 0.6;

                m[(i + 1) * 9 + (j + 1) * 3 + (k + 1)][0] = 1;
                m[(i + 1) * 9 + (j + 1) * 3 + (k + 1)][1] = x.x;
                m[(i + 1) * 9 + (j + 1) * 3 + (k + 1)][2] = x.y;
                m[(i + 1) * 9 + (j + 1) * 3 + (k + 1)][3] = x.z;

                f[(i + 1) * 9 + (j + 1) * 3 + (k + 1)] = getDist(x);
               
            }
        }

    }    
    
        float4 val = float4(getDist(xi), getNormal(xi, sdf));
    tex3D_uav[threadId.xyz] = val;
}
