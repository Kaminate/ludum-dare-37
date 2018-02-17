#pragma pack_matrix( row_major )

// IMPORTANT:
//   A lot of this shit has to match text.fx

Texture2D txDiffuse : register( t0 );
SamplerState LinSampler : register( s0 );

cbuffer DataConstantBuffer : register( b0 )
{
  matrix world;
  matrix view;
  float4 color;
  float2 uvMin;
  float2 uvMax;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};
 
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};


PS_INPUT vsmain( VS_INPUT input )
{
  float4 Pos = input.Pos;
  Pos = mul( world, Pos );
  Pos = mul( view, Pos );

  PS_INPUT output;
  output.Pos = Pos;
  output.Tex = lerp( uvMin, uvMax, input.Tex ); //* ( uvMax - uvMin );
  return output;
}

float4 psmain( PS_INPUT input) : SV_Target
{
  return txDiffuse.Sample( LinSampler, input.Tex ) * color;
}
