#include "Header.fx"

cbuffer VS_Ocean
{
	float WaveFrequency;
	float WaveAmplitude;

	float2 TextureScale;

	float2 BumpSpeed;
	float BumpHeight;
	float RunningTime; //�ð���
};

cbuffer PS_Ocean
{
	float4 ShallowColor; //������
	float4 DeepColor; //������

	float FresnelBias; //����
	float FresnelPower; //����
	float FresnelAmount; //������
	float ShoreBlend; //���İ� ��ȭ����

	float2 OceanSize; //���ؽ��� ��üũ��
	float HeightRatio; //���� ������

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