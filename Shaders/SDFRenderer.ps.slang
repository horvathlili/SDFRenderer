#include "../Utils/Ray.hlsl"
#include "../Utils/sdf.hlsl"

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
    float3 boundingBox;
};

Texture3D texture;
SamplerState mSampler;

bool isOutOfBox(float3 pos)
{
    bool r = (pos.x > -boundingBox.x / 2.0 && pos.x < boundingBox.x / 2.0 &&
              pos.y > -boundingBox.y / 2.0 && pos.y < boundingBox.y / 2.0 &&
              pos.z > -boundingBox.z / 2.0 && pos.z < boundingBox.z / 2.0);

    return !r;

}

float get_texture(float3 uvw)
{
    return texture.Sample(mSampler, uvw).r;
}

float3 get_normal(float3 uvw)
{
    return texture.Sample(mSampler, uvw).gba;
}


float4 main(psIn input) : SV_TARGET
{
    Ray r = getRay(eye.xyz, center.xyz, up.xyz, ar, input.pixelpos.x, input.pixelpos.y);  
    
    float t = 0.0f;
    int i;
    int n = 50;
    float3 pos;
    
    for (i = 0; i < n; i++)
    {
        pos = r.p + t * r.v;
        //float h = map(pos,sdf);
        float h = get_texture(pos / (float3) boundingBox + 0.5);
        if (h < 0.001 || isOutOfBox(pos))
            break;
        t += h;
    }

    if (i >= n || isOutOfBox(pos))
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
        float3 diffuse = clamp(dot(normalize(light_dir), get_normal(pos / (float3) boundingBox + 0.5)), 0, 1) * Ld * Kd;

        return float4(ambient+diffuse, 1);
    }

}
