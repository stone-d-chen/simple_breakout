cbuffer constants : register(b0)
{
   row_major float4x4 transform;
   row_major float4x4 projection;
}

struct vs_in
{
   float2 position : POS;
   float2 texcoord : TEX;
};

struct vs_out
{
   float4 position : SV_POSITION;
   float2 texcoord : TEX;
};

Texture2D    mytexture : register(t0);
SamplerState mysampler : register(s0);

vs_out vs_main(vs_in input)
{
   vs_out output;

    // output.position = mul(float4(input.position, 4.0f, 1.0f), mul(projection, transform));
    output.position = mul(float4(input.position, 4.0f, 1.0f), mul(transform, projection));
    
   // output.position = mul(float4(input.position, 4.0f ,0.0f), mul(projection, transform));
    
   // output.position = mul(float4(input.position, 0.5f, 1.0f), mul(transform, projection));

   output.texcoord = input.texcoord;

   return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
    return mytexture.Sample(mysampler, input.texcoord);
}