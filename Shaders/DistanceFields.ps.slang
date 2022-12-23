#ifndef TEXORDER
#define TEXORDER 0
#endif

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

cbuffer psCb
{
    float4 eye;
    float4 center;
    float4 up;
    float ar;
    int sdf;
    int texorder;
    int res;
    float boundingBox;
    float4x4 viewproj;
    bool box;
};

Texture3D texture1;
Texture3D texture2;
Texture3D texture3;
SamplerState mSampler;

bool isOutOfBox(float3 pos)
{
    bool r = (pos.x > -boundingBox / 2.0 && pos.x < boundingBox / 2.0 &&
        pos.y > -boundingBox / 2.0 && pos.y < boundingBox / 2.0 &&
        pos.z > -boundingBox / 2.0 && pos.z < boundingBox / 2.0);

    return !r;

}

float get_texture(float3 uvw, float3 pos)
{
#if TEXORDER == 0
        return texture1.Sample(mSampler, uvw).r;
#endif
#if TEXORDER == 1  
        float4 s = texture1.Sample(mSampler, uvw);
        return s.r*pos.x+s.g*pos.y+s.b*pos.z+s.a;
#endif
#if TEXORDER == 2
        float4 s1 = texture1.Sample(mSampler, uvw);
        float4 s2 = texture2.Sample(mSampler, uvw);
        float4 s3 = texture3.Sample(mSampler, uvw);
        return s1.r * pos.x + s1.g * pos.y + s1.b * pos.z
             + s1.a * pos.x*pos.x + s2.r * pos.y*pos.y + s2.g * pos.z*pos.z
             + s2.b * pos.x*pos.y + s2.a*pos.x*pos.z + s3.r*pos.y*pos.z + s3.g;
#endif
}


float3 get_normal(float3 uvw)
{
    #if TEXORDER == 0
        return texture1.Sample(mSampler, uvw).gba;
    #endif
    #if TEXORDER == 1
        return texture1.Sample(mSampler, uvw).rgb;
    #endif
    #if TEXORDER == 2
        return texture1.Sample(mSampler, uvw).rgb;
    #endif
}


PsOut main(psIn input) : SV_TARGET
{
    PsOut o;
    Ray r;
    if (!box)
    {
        r = getRay(eye.xyz, center.xyz, up.xyz, ar, input.pixelpos.x, input.pixelpos.y);
    }
    else
    {
        r.v = normalize(input.pixelpos - eye.xyz);
        r.p = input.pixelpos + 0.001 * r.v;
        
    }
        float t = 0.0f;
        int i;
        int n = 100;
        float3 pos = r.p + t * r.v;
    //return texture.Sample(mSampler, pos);
    
        for (i = 0; i < n; i++)
        {
            pos = r.p + t * r.v;
        float h = get_texture(pos / float3(boundingBox + boundingBox/(float) res) + float3(0.5), pos);
        if (h < 0.0001 || isOutOfBox(pos))
                break;
            t += h;
        }
    
        if (i >= n || isOutOfBox(pos))
        {
            o.col =  float4(0, 0, 0, 1);
        }
        else
        {
            float3 La = float3(0.4, 0.4, 0.4);
            float3 Ld = float3(0.8, 0.8, 0.8);
        
            float3 Ka = float3(0.2, 0.4, 0.6);
            float3 Kd = float3(0.8, 0.8, 0.8);
            
            float3 light_dir = float3(-1, -1, -1);
        
            float3 ambient = La * Ka;
            float3 diffuse = clamp(dot(normalize(-light_dir), get_normal(pos / boundingBox + 0.5)), 0, 1) * Ld * Kd;

           o.col =  float4(ambient + diffuse, 1);
        }
    
    // calc fragment depth
    float4 depth_vec = mul(float4(pos, 1), viewproj);
    o.depth = depth_vec.z / depth_vec.w;
    
     return o;
}

