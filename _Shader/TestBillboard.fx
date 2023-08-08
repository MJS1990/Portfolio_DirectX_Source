#include "Header.fx"
//-----------------------------------------------------------------------------
// VertexShader
//-----------------------------------------------------------------------------
//struct VertexInput
//{
//	float4 Position : POSITION0;
//	float2 Size : SIZE0;
//};

struct VertexOutput
{
    float4 Center : POSITION0;
	float2 Size : SIZE0;
};

VertexOutput VS(VertexSize input)
{
    VertexOutput output;

	output.Center = input.Position;
	output.Size = input.Size;

    return output;
}

struct GeometryOutput
{
	float4 Position : SV_POSITION;
	float2 Uv : UV0;
};

static const float2 TexCoord[4] =
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 1.0f),
	float2(1.0f, 0.0f)
};

[maxvertexcount(4)]
void GS(point VertexOutput input[1], inout TriangleStream<GeometryOutput> stream)
{
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = ViewInverse._41_42_43 - input[0].Center; //ViewInverse._14_24_34
	look.y = 0.0f; //수직 고정
	look = normalize(look);

	float3 right = cross(up, look);

	float halfWidth = 0.5f * input[0].Size.x;
	float halfHeight = 0.5f * input[0].Size.y;

	float4 v[4];
	v[0] = float4(input[0].Center + halfWidth * right - halfHeight * up, 1.0f);
	v[1] = float4(input[0].Center + halfWidth * right + halfHeight * up, 1.0f);
	v[2] = float4(input[0].Center - halfWidth * right - halfHeight * up, 1.0f);
	v[3] = float4(input[0].Center - halfWidth * right + halfHeight * up, 1.0f);

	GeometryOutput output;
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		output.Position = mul(v[i], View);
		output.Position = mul(output.Position, Projection);
		
		output.Uv = TexCoord[i];
		
		stream.Append(output);
	}
}

//-----------------------------------------------------------------------------
// PixelShader
//-----------------------------------------------------------------------------
//SamplerState Sampler
//{
//    Filter = MIN_MAG_MIP_LINEAR;
//    AddressU = Wrap;
//    AddressV = Wrap;
//};

Texture2D BillboardTex;
SamplerState BillboardSampler;

float4 PS(GeometryOutput input) : SV_TARGET
{
	float4 color = BillboardTex.Sample(BillboardSampler, input.Uv);
	clip(color.a - 0.5f);

    return color;
}

//-----------------------------------------------------------------------------
// Technique
//-----------------------------------------------------------------------------
technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(CompileShader(gs_5_0, GS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}