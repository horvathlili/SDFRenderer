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
    if (sdf == 1)
        return sdSphere(p, 1);
    if (sdf == 2)
        return sdTorus(p, float2(1, 0.3));
    if (sdf == 3)
        return sdBoxFrame(p, float3(1, 1, 1), 0.1);

    return 0;
}

float3 getNormal(float3 p, int sdf)
{
    float e = 0.0001; // or some other value
    float3 n;
    n.x = map(p + float3(e, 0, 0), sdf) - map(p - float3(e, 0, 0),sdf);
    n.y = map(p + float3(0, e, 0), sdf) - map(p - float3(0, e, 0), sdf);
    n.z = map(p + float3(0, 0, e), sdf) - map(p - float3(0, 0, e), sdf);

    return normalize(n);

}
