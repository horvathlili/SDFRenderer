#include "../Utils/Ray.hlsl"
#include "../Utils/sdf.hlsl"

struct psIn
{
    float4 pos : SV_POSITION;
    float3 col : COLOR;
    float3 pixelpos : POSITION;
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

    bool box;
};

Texture3D texture;
SamplerState mSampler;

bool isOutOfBox(float3 pos)
{
    bool r = (pos.x > -boundingBox / 2.0 && pos.x < boundingBox / 2.0 &&
        pos.y > -boundingBox / 2.0 && pos.y < boundingBox / 2.0 &&
        pos.z > -boundingBox / 2.0 && pos.z < boundingBox / 2.0);

    return !r;

}

float get_texture(float3 uvw, float3 pos, int order)
{
    if (order == 0)
   {
        return texture.Sample(mSampler, uvw).r;
    }
    if (order == 1)
    {     
        float4 s = texture.Sample(mSampler, uvw);
        return s.r*pos.x+s.g*pos.y+s.b*pos.z+s.a;
    }
    return 0;
}


float3 get_normal(float3 uvw, int order)
{
    if (order == 0)
        return texture.Sample(mSampler, uvw).gba;
    if (order == 1)
        return texture.Sample(mSampler, uvw).rgb;
    return 0;
}

float4 main(psIn input) : SV_TARGET
{
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
        float h = get_texture(pos / float3(boundingBox + boundingBox/2.0/(float) res) + float3(0.5), pos/boundingBox*2.0, texorder) * boundingBox/2;
        if (h < 0.0001 || isOutOfBox(pos))
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
        float3 diffuse = clamp(dot(normalize(-light_dir), get_normal(pos / boundingBox + 0.5, texorder)), 0, 1) * Ld * Kd;

            return float4(ambient + diffuse, 1);
        }
}
