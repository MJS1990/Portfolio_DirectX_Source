#include "Header.fx"

cbuffer VS_Ocean
{
	float WaveFrequency;
	float WaveAmplitude;

	float2 TextureScale;

	float2 BumpSpeed;
	float BumpHeight;
	float RunningTime; //시간값
};

cbuffer PS_Ocean
{
	float4 ShallowColor; //얕은색
	float4 DeepColor; //깊은색

	float FresnelBias; //편향
	float FresnelPower; //강도
	float FresnelAmount; //보정값
	float ShoreBlend; //알파값 변화정도

	float2 OceanSize; //버텍스의 전체크기
	float HeightRatio; //높이 보정값

	float Padding;
}

struct VertexOutput
{
	float4 Position : SV_POSITION0;
	float2 Uv : UV0;
};

VertexOutput VS(VertexTexture input)
{
	VertexOutput output;

	output.Position = mul(input.Position, World);
	output.Position = mul(output.Position, View);
	output.Position = mul(output.Position, Projection);

	output.Uv = input.Uv;

	return output;
}

Texture2D HeightMap;
SamplerState HeightSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};
SamplerState NormalSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PS(VertexOutput input) : SV_TARGET
{
	return float4(1, 0, 0, 1);
}

RasterizerState FillMode
{
	FillMode = Wireframe;

	AntialiasedLineEnable = FALSE;
	CullMode = BACK;
	DepthBias = 0;
	DepthBiasClamp = 0.0f;
	DepthClipEnable = TRUE;
	FrontCounterClockWise = FALSE;
	MultisampleEnable = FALSE;
	ScissorEnable = FALSE;
	SlopeScaledDepthBias = 0.0f;
};

technique11 T0
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));

		SetRasterizerState(FillMode);
	} //;
} //;