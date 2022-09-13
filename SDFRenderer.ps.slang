#include "Ray.hlsl"
#include "sdf.hlsl"

struct psIn
{
    float4 pos : SV_POSITION;
    float3 col : COLOR;
    float2 pixelpos : POSITION;
};

cbuffer psCb
{
    float4 eye;
    float4 center;
    float4 up;
    float ar;
    int sdf;
};


float4 main(psIn input) : SV_TARGET
{
    Ray r = getRay(eye.xyz, center.xyz, up.xyz, ar, input.pixelpos.x, input.pixelpos.y);  
    
    float t = 0.0f;
    int i;
    int n = 200;
    
    for (i = 0; i < n; i++)
    {
        float3 pos = r.p + t * r.v;
        float h = map(pos,sdf);
        if (h < 0.001)
            break;
        t += h;
    }

    if (i >= n)
    {
        return float4(0, 0, 0, 1);
    }
    else
    {
        float3 La = float3(0.4, 0.4, 0.4);
        float3 Ld = float3(0.8, 0.8, 0.8);
        
        float3 Ka = float3(0.2, 0.4, 0.6);
        float3 Kd = float3(0.8, 0.8, 0.8);
            
        float3 light_dir = float3(-1, -1, -1);
        
        float3 ambient = La * Ka;
        float3 diffuse = clamp(dot(normalize(light_dir), getNormal(r.p+t*r.v, sdf)), 0, 1) * Ld * Kd;

        return float4(ambient+diffuse, 1);
    }
}
