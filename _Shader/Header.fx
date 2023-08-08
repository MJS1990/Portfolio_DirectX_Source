//-----------------------------------------------------------------------------
// CBuffer
//-----------------------------------------------------------------------------
cbuffer CB_PerFrame //매 프레임 별로 바뀔 데이터
{
    matrix View;
	matrix ViewInverse; //추가
    float3 ViewDirection;

    float Time;
    float3 ViewPosition;
};

cbuffer CB_Projection
{
    matrix Projection;
};

cbuffer CB_World
{
    matrix World;
};

cbuffer CB_Light
{
    float4 LightAmbient;
    float4 LightDiffuse;
    float4 LightSpecular;
    float3 LightDirection;
	float3 LightPosition;
}

cbuffer CB_Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float Shininess;
}

//-----------------------------------------------------------------------------
// Textures
//-----------------------------------------------------------------------------
Texture2D DiffuseMap;
Texture2D SpecularMap;
Texture2D NormalMap;

//-----------------------------------------------------------------------------
// Vertex Layout
//-----------------------------------------------------------------------------
struct Vertex
{
    float4 Position : POSITION0;
};

struct VertexSize
{
    float4 Position : POSITION0;
    float2 Size : SIZE0;
};

struct VertexColor
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
};

struct VertexColorNormal
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
    float3 Normal : NORMAL0;
};

struct VertexTexture
{
    float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
};

struct VertexTextureNormal
{
    float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
};

struct VertexTextureNormalTangent
{
    float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
};

struct VertexTextureNormalTangentBlend
{
    float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
    float4 BlendIndices : BLENDINDICES0;
    float4 BlendWeights : BLENDWEIGHTS0;
};

struct VertexColorTextureNormal
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL0;
};


//-----------------------------------------------------------------------------
// Bones
//-----------------------------------------------------------------------------
int BoneIndex;
cbuffer CB_Bones
{
    matrix Bones[128];
};

matrix BoneWorld()
{
    return Bones[BoneIndex];
}

matrix SkinWorld(float4 blendIndices, float4 blendWeights)
{
    float4x4 transform = 0;
    transform += mul(blendWeights.x, Bones[(uint) blendIndices.x]);
    transform += mul(blendWeights.y, Bones[(uint) blendIndices.y]);
    transform += mul(blendWeights.z, Bones[(uint) blendIndices.z]);
    transform += mul(blendWeights.w, Bones[(uint) blendIndices.w]);

    return transform;
}

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------
float3 WorldNormal(float3 normal)
{
    return normalize(mul(normal, (float3x3) World));
}

//-----------------------------------------------------------------------------
// Lighting
//-----------------------------------------------------------------------------
struct Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float Shininess;
};

//-----------------------------------------------------------------------------
// Directional Lighting
//-----------------------------------------------------------------------------
struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
};

void ComputeDirectionalLight(Material m, DirectionalLight l, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0, 0, 0, 0);
    diffuse = float4(0, 0, 0, 0);
    specular = float4(0, 0, 0, 0);

    float3 light = -l.Direction;
    ambient = m.Ambient * l.Ambient;

    float diffuseFactor = dot(light, normal);

    [flatten]
    if (diffuseFactor > 0.0f)
    {
        diffuse = diffuseFactor * m.Diffuse * l.Diffuse;

        
        float3 r = reflect(-light, normal);
        
        float specularFactor = 0;
        specularFactor = saturate(dot(r, toEye));
        specularFactor = pow(specularFactor, m.Specular.a);
        specular = specularFactor * m.Specular * l.Specular;
    }
}

//-----------------------------------------------------------------------------
// Point Lighting
//-----------------------------------------------------------------------------
struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;

    float3 Attenuation;
    float PointLight_Padding;
};

cbuffer CB_PointLight
{
    PointLight PointLights[16];
    int PointLightCount;
};

void ComputePointLight(Material m, PointLight l, float3 position, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0, 0, 0, 0);
    diffuse = float4(0, 0, 0, 0);
    specular = float4(0, 0, 0, 0);

    float3 light = l.Position - position;
    float dist = length(light);
    
    if (dist > l.Range)
        return;

    light /= dist;
    ambient = m.Ambient * l.Ambient;

    float diffuseFactor = dot(light, normal);

    [flatten]
    if (diffuseFactor > 0.0f)
    {
        diffuse = diffuseFactor * m.Diffuse * l.Diffuse;

        float3 r = reflect(-light, normal);
        
        float specularFactor = 0;
        specularFactor = saturate(dot(r, toEye));
        specularFactor = pow(specularFactor, m.Specular.a);
        specular = specularFactor * m.Specular * l.Specular;
    }


    float attenuate = 1.0f / dot(l.Attenuation, float3(1.0f, dist, dist * dist));

    diffuse *= attenuate;
    specular *= attenuate;
}

//-----------------------------------------------------------------------------
// Spot Lighting
//-----------------------------------------------------------------------------
struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float PointLight_Padding;

    float3 Direction;
    float Spot;

    float3 Attenuation;
    float PointLight_Padding2;
};

cbuffer CB_SpotLight
{
    SpotLight SpotLights[16];
    int SpotLightCount;
};

void ComputeSpotLight(Material m, SpotLight l, float3 position, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0, 0, 0, 0);
    diffuse = float4(0, 0, 0, 0);
    specular = float4(0, 0, 0, 0);

    float3 light = l.Position - position;
    float dist = length(light);

    light /= dist;
    ambient = m.Ambient * l.Ambient;

    float diffuseFactor = dot(light, normal);

    [flatten]
    if (diffuseFactor > 0.0f)
    {
        diffuse = diffuseFactor * m.Diffuse * l.Diffuse;

        float3 r = reflect(-light, normal);
        
        float specularFactor = 0;
        specularFactor = saturate(dot(r, toEye));
        specularFactor = pow(specularFactor, m.Specular.a);
        specular = specularFactor * m.Specular * l.Specular;
    }


    float spot = pow(max(dot(-light, l.Direction), 0.0f), l.Spot);
    float attenuate = spot / dot(l.Attenuation, float3(1.0f, dist, dist * dist));

    ambient *= attenuate;
    diffuse *= attenuate;
    specular *= attenuate;
}

//추가///////////////////////////////////////////////////////
//디퓨즈 라이팅
void DiffuseLighting(inout float4 color, float3 normal)
{
	float intensity = saturate(dot(normal, -LightDirection));

	color = color + LightDiffuse * intensity;
}

void DiffuseLighting(inout float4 color, float4 diffuse, float3 normal)
{
	float intensity = saturate(dot(normal, -LightDirection)); //각도에 따른 가중치(비율)
    
	color = color + LightDiffuse * diffuse * intensity;
}

void TessDiffuseLighting(inout float4 color, float4 diffuse, float3 normal)
{
    float lightNormal = saturate(dot(normal, -LightDirection));
    float tempNormal = saturate(normal);
    float intensity = 0.0f;

    if(lightNormal >= tempNormal)
    {
        intensity = 1.0f;
    }
    else
    {
        intensity = saturate(lightNormal); //각도에 따른 가중치(비율)
    }
    
    color = color + LightDiffuse * diffuse * intensity;
}

//스페큘러 라이팅
void SpecularLighting(inout float4 color, float3 normal, float3 viewDirection)
{
	float3 reflection = reflect(LightDirection, normal);
	float intensity = saturate(dot(reflection, viewDirection));
	float specular = pow(intensity, Shininess);

	color = color + Specular * specular;
}

void SpecularLighting(inout float4 color, float4 specularMap, float3 normal, float3 viewDirection)
{
	float3 reflection = reflect(LightDirection, normal);
	float intensity = saturate(dot(reflection, viewDirection));
	float specular = pow(intensity, Shininess);

	color = color + Specular * specular * specularMap;
}
//노멀 매핑
void NormalMapping(inout float4 color, float4 normalMap, float3 normal, float3 tangent)
{
	float3 N = normal; //Z
	float3 T = normalize(tangent - dot(tangent, N) * N); //X
	float3 B = cross(T, N); //Y

	float3x3 TBN = float3x3(T, B, N);

	float3 coord = 2.0f * normalMap - 1.0f;
	float3 bump = mul(coord, TBN);

	float intensity = saturate(dot(bump, -LightDirection));
	color = color * intensity; 
}

float3 WorldTangent(float3 tangent, matrix world)
{
	tangent = normalize(mul(tangent, (float3x3)world));

	return tangent;
}