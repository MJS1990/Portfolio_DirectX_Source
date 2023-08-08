#include "Header.fx"
//-----------------------------------------------------------------------------
// VertexShader
//-----------------------------------------------------------------------------
struct VertexInput
{
	float4 Position : POSITION0;
	float2 Size : SIZE0;
	float3 Instance : INSTANCE0;
	uint InstanceId : SV_INSTANCEID0; //인스턴스 인덱스(시스템 예약어)
};

struct VertexOutput
{
    float4 Center : POSITION0;
	float2 Size : SIZE0;
	uint Id : INSTANCE0;
};

struct GeometryOutput
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL0;
	float2 Uv : UV0;
};

static const float2 TexCoord[4] =
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 1.0f),
	float2(1.0f, 0.0f)
};

Texture2D BillboardTex;
SamplerState BillboardSampler;

VertexOutput VS(VertexInput input)
{
    VertexOutput output;

	//Instance객체로 Test
	output.Center.x = input.Position.x + input.Instance.x;
	output.Center.y = input.Position.y + input.Instance.y;
	output.Center.z = input.Position.z + input.Instance.z;
	output.Center.w = 1;


	output.Size = input.Size;
	output.Id = input.InstanceId;

    return output;
}

[maxvertexcount(4)]
void GS(point VertexOutput input[1], inout TriangleStream<GeometryOutput> stream)
{
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = ViewInverse._41_42_43 - input[0].Center;
	look.y = 0.0f; //수직 고정
	look = normalize(look);

	float3 right = cross(up, look);

	float halfWidth = 0.5f * input[0].Size.x;
	float halfHeight = 0.5f * input[0].Size.y;

	float4 v[4];
	v[0] = float4(input[0].Center + halfWidth * right - halfHeight * up, 1.0f); //2
	v[1] = float4(input[0].Center + halfWidth * right + halfHeight * up, 1.0f) + (cos(Time) * 0.001f); //3
	v[2] = float4(input[0].Center - halfWidth * right - halfHeight * up, 1.0f); //0
	v[3] = float4(input[0].Center - halfWidth * right + halfHeight * up, 1.0f) + (cos(Time) * 0.001f); //1

	GeometryOutput output;
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		output.Position = mul(v[i], World);

		output.Position = mul(v[i], View);
		output.Position = mul(output.Position, Projection);
		
		output.Normal = ViewInverse._41_42_43;
		output.Uv = TexCoord[i];
		
		stream.Append(output);
	}
}

//-----------------------------------------------------------------------------
// PixelShader
//-----------------------------------------------------------------------------
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