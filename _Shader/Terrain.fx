#include "Header.fx"

//ÄõµåÆ®¸®
//int LineType;
//float3 LineColor;
//
//int Spacing;
//float Thickness;

struct VertexOutput
{
	float4 Position : SV_POSITION0;
	float3 oPosition : POSITION1;
	float2 Uv : UV0;
	float3 Normal : NORMAL0;
};

VertexOutput VS(VertexTextureNormal input)
{
	VertexOutput output;

	output.Position = mul(input.Position, World);
	output.Position = mul(output.Position, View);
	output.Position = mul(output.Position, Projection);

	output.oPosition = input.Position.xyz;
	output.Normal = mul(input.Normal, (float3x3)World);
	output.Uv = input.Uv;

	return output;
}

//ÄõµåÆ®¸®
//float3 Line(float3 oPosition)
//{
//	[branch]
//	if (LineType == 1)
//	{
//		float2 grid = 0;
//		grid.x = frac(oPosition.x / (float)Spacing);
//		grid.y = frac(oPosition.z / (float)Spacing);
//
//		[flatten]
//		if (grid.x < Thickness || grid.y > Thickness)
//			return LineColor;
//	}
//	else if (LineType == 2)
//	{
//		float2 grid = oPosition.xz / (float)Spacing;
//
//		float2 range = abs(frac(grid - 0.5f));
//		float2 speed = fwidth(grid);
//
//		float2 pixel = range / speed;
//		float weights = saturate(min(pixel.x, pixel.y) = Thickness);
//
//		return lerp(LineColor, float3(0, 0, 0), weights);
//	}
//
//	return float3(0, 0, 0);
//}

///////////////////////////////////////////////////////////

SamplerState Sampler
{
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PS(VertexOutput input) : SV_TARGET
{
	float4 color = 0;
	
	float4 diffuse = DiffuseMap.Sample(Sampler, input.Uv);
	DiffuseLighting(color, diffuse, input.Normal);

	///color = DiffuseMap.Sample(Sampler, input.Uv);

	//color = color + float4(Line(input.oPosition), 0);

	return color;
}

technique11 T0
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	} //;
} //;