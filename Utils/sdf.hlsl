#ifndef SDF
#define SDF 1
#endif
#include "teapot.slang"

float sdSphere(float3 p, float s)
{
    return length(p) - s;
}

float sdTorus(float3 p, float2 t)
{
    float2 q = float2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

float sdBoxFrame(float3 p, float3 b, float e)
{
    p = abs(p) - b;
    float3 q = abs(p + e) - e;
    return min(min(
      length(max(float3(p.x, q.y, q.z), 0.0)) + min(max(p.x, max(q.y, q.z)), 0.0),
      length(max(float3(q.x, p.y, q.z), 0.0)) + min(max(q.x, max(p.y, q.z)), 0.0)),
      length(max(float3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0));
}

float map(float3 p, int sdf)
{
    #if SDF == 1
       return sdSphere(p, 0.3);
    #endif
    #if SDF == 2
        return sdTorus(p, float2(0.3, 0.1));
    #endif
    #if SDF == 3
        return sdBoxFrame(p, float3(0.2, 0.2, 0.2), 0.05);
    #endif
    #if SDF == 4
        return funDist(p);
    #endif
}

float sdTorus2(float3 p, float r, float R, float3 c, float3 d)
{
    float3 p2 = p - c;
    float3 e2 = normalize(d);
    float3 n = cross(e2, normalize(p2));

    float3 e1 = normalize(cross(e2, n));

    float px = dot(e1, p2);
    float py = dot(e2, p2);

    float dist1 = sqrt((px - R) * (px - R) + py * py) - r;
    float dist2 = sqrt((px + R) * (px + R) + py * py) - r;

    if (abs(dist1) <= abs(dist2))
        return dist1;
    else
        return dist2;
}

float3 getNormal(float3 p, int sdf)
{
    float e = 0.0001;
    float3 n;
    n.x = map(p + float3(e, 0, 0), sdf) - map(p - float3(e, 0, 0),sdf);
    n.y = map(p + float3(0, e, 0), sdf) - map(p - float3(0, e, 0), sdf);
    n.z = map(p + float3(0, 0, e), sdf) - map(p - float3(0, 0, e), sdf);

    return normalize(n);

}
