#ifndef MODE
#define MODE 0
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
    float r1;
    float R1;
    float3 c1;
    float3 d1;
    float4 viewproj;
    int cluster;
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
        float h;
        #if MODE == 0
            h = sdTorus2(pos, r, R, c, d.xzy);
        #endif
        #if MODE == 1
            h = min(sdTorus2(pos, r, R, c, d.xzy),sdTorus2(pos, r1, R1, c1, d1.xzy));
        #endif
        #if MODE == 2
            if (cluster == 0){
                h = sdTorus2(pos, r, R, c, d.xzy);
            }
            else
            {
                h = sdTorus2(pos, r1, R1, c1, d1.xzy);
            }
        #endif
        if (h < 0.0001)
            break;
        t += h;
    }

    if (i < n)
        o.col = float4(1, 1, 1, 1);

    #if MODE == 1
        for (int j = 0; j < 4*4*4; j++){
            if (length(t*ray.v) > length(float3(points[j].xyz)-ray.p) || i >= n){
                if (dot(ray.v,normalize(float3(points[j].xyz)-ray.p)) >0.9999){
                o.col = float4(1,0,0,1);
            }
            }
        }
    #endif
    
  
    // calc fragment depth
    float4 depth_vec = mul(float4(t * ray.v + ray.p, 1), viewproj);
    o.depth = depth_vec.z / depth_vec.w;
    o.depth = 1;

    return o;
}
