


float sampleSdf(float3 uvw, float3 pos, Texture3D texture, SamplerState mSampler)
{
#if SAMPLING == 0
   normalSampling(float3 uvw, float3 pos, Texture3D texture, SamplerState mSampler)
#endif
    return 0;
}


float normalSampling(float3 uvw, float3 pos, Texture3D texture, SamplerState mSampler)
{
    #if ORDER == 0
        return texture.Sample(mSampler, uvw).r;
    #endif
    #if ORDER == 1
        float4 s = texture.Sample(mSampler, uvw);
        return dot(s, float4(pos, 1.0));
    #endif
    return 0;
}
