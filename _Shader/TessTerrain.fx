#include "Header.fx"

cbuffer CB_Terrain
{
    float4 FogColor;
    float FogStart;
    float FogRange;
 
    float MaxDistance;
    float MinDistance;
    float MaxTessellation; //500
    float MinTessellation; //20

    float TexelCellSpaceU; //1.0f / terrain->Desc().HeightMapWidth;
    float TexelCellSpaceV; //1.0f / terrain->Desc().HeightMapHeight;
    float WorldCellSpace; //0.5f

    float2 TexScale = 66.0f; //높이맵용
    ///float2 TexScale; //높이맵용
    float CB_Terrain_Padding; //TODO : 패딩 빼도 되는지 확인

    float4 WorldFrustumPlanes[6]; //world변환된 frustum
};

//-----------------------------------------------------------------------------
// VertexShader
//-----------------------------------------------------------------------------

Texture2D HeightMap;
SamplerState HeightMapSampler
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;
};

struct VertexInput
{
    float4 Position : POSITION0;
    float2 Uv : UV0;
    float2 BoundsY : BOUNDY0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
};

struct VertexOutput
{
	float4 Position : SV_POSITION0;
    float2 Uv : UV0;
    float2 BoundsY : BOUNDY0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
};

//노멀벡터 계산함수
float3 CalcNormal(float3 pos0, float3 pos1, float3 pos2)
{
    float3 d1 = pos1 - pos0;
    float3 d2 = pos2 - pos0;

    float3 normal = cross(d1, d2);
    //normalize(normal);
    //saturate(normal);

    return normal;
}

VertexOutput VS(VertexInput input)
{
	VertexOutput output;
    output.Position = input.Position;
    output.Position.y = HeightMap.SampleLevel(HeightMapSampler, input.Uv, 0).r * 8.0f;
   
    output.Uv = input.Uv;
    output.BoundsY = input.BoundsY;

    output.Normal = input.Normal;
    output.Tangent = input.Tangent;

	return output;
}

//-----------------------------------------------------------------------------
// Constant Hull Shader
//-----------------------------------------------------------------------------
float CalcTessFactor(float3 position)
{
    //거리계산
    float d = distance(position, ViewPosition);
    float s = saturate((d - MinDistance) / (MaxDistance - MinDistance));
    
    return pow(2, lerp(MaxTessellation, MinTessellation, s));
}

bool AabbBehindPlaneTest(float3 center, float3 extents, float4 plane) //범위 뒤에 있는지 판단
{
    float3 n = abs(plane.xyz);
    float r = dot(extents, n);
    float s = dot(float4(center, 1), plane);

    return (s + r) < 0.0f;
}

bool AabbOutsideFrustumTest(float3 center, float3 extents)
{
    [unroll]
    for (int i = 0; i < 6; i++)
    {
        [flatten]
        if (AabbBehindPlaneTest(center, extents, WorldFrustumPlanes[i]));
            return true;
    }

    return false;
}

struct ConstantOutput
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;

    //float3 Normal : NORMAL0;
    //float3 Tangent : TANGENT0;
};

ConstantOutput HS_Constant(InputPatch<VertexOutput, 4> input, uint patchID : SV_PrimitiveId)
{
    ConstantOutput output;

    float minY = input[0].BoundsY.x; //해당 정점이 속한 셀의 최소높이값
    float maxY = input[0].BoundsY.y; //해당 정점이 속한 셀의 최대높이값

    float3 vMin = float3(input[2].Position.x, minY, input[2].Position.z);
    float3 vMax = float3(input[1].Position.x, maxY, input[1].Position.z);

    float3 boxCenter = (vMin + vMax) * 0.5f;
    float3 boxExtents = (vMax + vMin) * 0.5f; //넓이

    ////frustum컬링->범위안에 없으면 테셀레이션 수행X
    //[flatten]
    //if (AabbOutsideFrustumTest(boxCenter, boxExtents));
    //{
    //    output.Edges[0] = 0.0f;
    //    output.Edges[1] = 0.0f;
    //    output.Edges[2] = 0.0f;
    //    output.Edges[3] = 0.0f;
    //
    //    output.Inside[0] = 0.0f;
    //    output.Inside[1] = 0.0f;
    //
    //    return output;
    //}

    //각 변의 중점
    float3 e0 = (input[0].Position + input[2].Position).xyz * 0.5f; 
    float3 e1 = (input[0].Position + input[1].Position).xyz * 0.5f;
    float3 e2 = (input[1].Position + input[3].Position).xyz * 0.5f;
    float3 e3 = (input[2].Position + input[3].Position).xyz * 0.5f;
    float3 c = (input[0].Position + input[1].Position + input[2].Position + input[3].Position).xyz * 0.25f;

    //노멀벡터 테스트
    float3 normal = CalcNormal(input[0].Position.xyz, input[1].Position.xyz, input[2].Position.xyz);
    /////////////////

    output.Edges[0] = CalcTessFactor(e0);
    output.Edges[1] = CalcTessFactor(e1);
    output.Edges[2] = CalcTessFactor(e2);
    output.Edges[3] = CalcTessFactor(e3);
    
    output.Inside[0] = CalcTessFactor(c);
    output.Inside[1] = output.Inside[0];

    //output.Normal[0] = input[0].Normal;
    //output.Normal[1] = input[1].Normal;
    //output.Normal[2] = input[2].Normal;
    //output.Normal[3] = input[3].Normal;
    //
    //output.Tangent[0] = input[0].Tangent;
    //output.Tangent[1] = input[1].Tangent;
    //output.Tangent[2] = input[2].Tangent;
    //output.Tangent[3] = input[3].Tangent;

    //output.Normal = input[0].Normal;
    //output.Tangent = input[0].Tangent;

    return output;
}

//-----------------------------------------------------------------------------
// ControlPoint Hull Shader
//-----------------------------------------------------------------------------
struct HullOutput
{
    float4 Position : POSITION0;
    float2 Uv : UV0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
};

[domain("quad")]
//[partitioning("integer")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_Constant")]
[maxtessfactor(64.0f)]
HullOutput HS(InputPatch<VertexOutput, 4> input, uint pointID : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HullOutput output;
    output.Position = input[pointID].Position;
    output.Uv = input[pointID].Uv;

    output.Normal = input[pointID].Normal;
    output.Tangent = input[pointID].Tangent;

    return output;
}

//-----------------------------------------------------------------------------
// DomainShader
//-----------------------------------------------------------------------------
struct DomainOutput
{
    float4 Position : SV_POSITION00;
    float3 wPosition : POSITION1;
    float2 Uv : UV0;
    float2 TileUv : UV1;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
};

[domain("quad")]
DomainOutput DS(ConstantOutput input, float2 uvw : SV_DomainLocation, const OutputPatch<HullOutput, 4> patch)
{
    DomainOutput output;

    ////CalcTempPosition
    //float3 tempPos[4];
    //tempPos[0] = patch[0].Position.xyz;
    //tempPos[1] = patch[1].Position.xyz;
    //tempPos[2] = patch[2].Position.xyz;
    //tempPos[3] = patch[3].Position.xyz;
    //
    //tempPos[0].y = HeightMap.SampleLevel(HeightMapSampler, patch[0].Uv, 0).r;// * 4.0f;
    //tempPos[1].y = HeightMap.SampleLevel(HeightMapSampler, patch[1].Uv, 0).r;// * 4.0f;
    //tempPos[2].y = HeightMap.SampleLevel(HeightMapSampler, patch[2].Uv, 0).r;// * 4.0f;
    //tempPos[3].y = HeightMap.SampleLevel(HeightMapSampler, patch[3].Uv, 0).r;// * 4.0f;
    ////////////////////

    float3 p0 = lerp(patch[0].Position, patch[1].Position, uvw.x).xyz;
    float3 p1 = lerp(patch[2].Position, patch[3].Position, uvw.x).xyz;
    float3 position = lerp(p0, p1, uvw.y);
    output.wPosition = position;

    float2 uv0 = lerp(patch[0].Uv, patch[1].Uv, uvw.x);
    float2 uv1 = lerp(patch[2].Uv, patch[3].Uv, uvw.x);
    output.Uv = lerp(uv0, uv1, uvw.y);

    //output.wPosition.y = HeightMap.SampleLevel(HeightMapSampler, output.Uv, 0).r * 4.0f;
    //output.wPosition.y = 1.0f;
    output.Position = mul(float4(output.wPosition, 1), View);
    output.Position = mul(output.Position, Projection);
   
    output.TileUv = output.Uv * TexScale; //TexScale 확인 필요


    //CalcNormalVector//////////////////////////////////////////////////////////////

    //output.Normal = input.Normal;
    //output.Tangent = input.Tangent;

    //float3 d1 = patch[1].Position - patch[0].Position;
    //float3 d2 = patch[2].Position - patch[0].Position;
    //float3 d1 = tempPos[1] - tempPos[0];
    //float3 d2 = tempPos[2] - tempPos[0];
    
    //float3 normal = cross(d1, d2);
    //normal = saturate(normal);

    //float3 n0 = lerp(patch[0].Normal, patch[1].Normal, uvw.x);
    //float3 n1 = lerp(patch[2].Normal, patch[3].Normal, uvw.x);
    //float3 normal = lerp(n0, n1, uvw.y);
    
    //Normal 이전 코드
    float3 normal = CalcNormal(patch[0].Position.xyz, patch[1].Position.xyz, patch[2].Position.xyz);
    
    output.Normal = mul(normal, (float3x3) World);

    //output.Normal = normal;
    //output.Tangent = patch[0].Tangent;
    ////////////////////////////////////////////////////////////////////////////////
    return output;
}

Texture2D TerrainMap;
Texture2D BlendMap;
Texture2D Normal;

SamplerState LinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;

	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PS(DomainOutput input, uniform bool fogEnabled) : SV_TARGET
{
    //주석은 텍스쳐배열 블랜딩용
    //인접픽셀 UV값
    float2 left = input.Uv + float2(-TexelCellSpaceU, 0.0f);
    float2 right = input.Uv + float2(TexelCellSpaceU, 0.0f);
    float2 top = input.Uv + float2(0.0f, -TexelCellSpaceV);
    float2 bottom = input.Uv + float2(0.0f, TexelCellSpaceV);
    //주변 픽셀의 높이값
    float leftY = HeightMap.SampleLevel(HeightMapSampler, left, 0).r;
    float rightY = HeightMap.SampleLevel(HeightMapSampler, right, 0).r;
    float topY = HeightMap.SampleLevel(HeightMapSampler, top, 0).r;
    float bottomY = HeightMap.SampleLevel(HeightMapSampler, bottom, 0).r;
    
    float3 tangent = normalize(float3(WorldCellSpace * 2.0f, rightY - leftY, 0.0f));
    float3 biTangent = normalize(float3(0.0f, bottomY - topY, WorldCellSpace * -2.0f));
    float3 normalW = cross(tangent, biTangent);
    
    float3 eye = ViewPosition - input.wPosition;
    float distanceToEye = length(eye);
    eye /= distanceToEye;
    
    //float4 c0 = LayerMapArray.Sample(LinearSampler, float3(input.TileUv, 0));
    //float4 c1 = LayerMapArray.Sample(LinearSampler, float3(input.TileUv, 1));
    //float4 c2 = LayerMapArray.Sample(LinearSampler, float3(input.TileUv, 2));
    //float4 c3 = LayerMapArray.Sample(LinearSampler, float3(input.TileUv, 3));
    //float4 c4 = LayerMapArray.Sample(LinearSampler, float3(input.TileUv, 4));
    //float4 t = BlendMap.Sample(LinearSampler, input.Uv);
    
    float4 c = TerrainMap.Sample(LinearSampler, input.TileUv * 16);
    
    //float4 color = c;
    float4 color = 0;
    //float4 color = c0;
    //color = lerp(color, c1, t.r);
    //color = lerp(color, c2, t.g);
    //color = lerp(color, c3, t.b);
    //color = lerp(color, c4, t.a);
    
    ////Calculate NormalVector
    ////중심차분법
    //float2 leftTex = input.Uv + float2(TexelCellSpaceU, 0.0f);
    //float2 rightTex = input.Uv + float2(TexelCellSpaceU, 0.0f);
    //float2 bottomTex = input.Uv + float2(0.0f, TexelCellSpaceU);
    //float2 topTex = input.Uv + float2(0.0f, TexelCellSpaceU);
    //
    //float leftY = HeightMap.Sample(HeightMapSampler, leftTex).r;
    //float rightY = HeightMap.Sample(HeightMapSampler, rightTex).r;
    //float bottomY = HeightMap.Sample(HeightMapSampler, bottomTex).r;
    //float topY = HeightMap.Sample(HeightMapSampler, topTex).r;

    //float3 tangent = normalize(float3(2.0f * WorldCellSpace, rightY - leftY, 0.0f));
    //float3 bitan = normalize(float3(0.0f, bottomY - topY, -2.0f * WorldCellSpace));
    //float3 normalW = cross(tangent, bitan);
    ////////////////////////

    //Test DiffuseLighting
    DiffuseLighting(color, c, normalW); //원본
    //TessDiffuseLighting(color, c, normalW); //테스트

    //Test NormalMapping
    float4 normal = NormalMap.Sample(LinearSampler, input.TileUv * 4);
    NormalMapping(color, normal, normalW, tangent);
     
    ///////////////////
    ////FOG
    //[flatten]
    //if (fogEnabled == true)
    //{
    //    float fogFactor = saturate((distanceToEye - FogStart) / FogRange);
    //
    //    color = lerp(color, FogColor, fogFactor);
    //}
    
    return color;
}

RasterizerState FillMode
{
    FillMode = Wireframe;
};

technique11 T0
{
	pass P0
	{
        //SetRasterizerState(FillMode);

		SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetPixelShader(CompileShader(ps_5_0, PS(true)));
    } //;
} //;