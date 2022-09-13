struct vsOut
{
    float4 pos : SV_POSITION;
    float3 col : COLOR;
    float2 pixelpos : POSITION;
};

struct vsIn
{
    float3 pos : POSITION;
    float3 col : COLOR;
};



vsOut main (vsIn input) {
  vsOut output;
    output.pos = float4(input.pos, 1);
    output.col = input.col;
    output.pixelpos = float2(input.pos.x, input.pos.y);
  return output;
}
