#include "Framework.h"
#include "Terrain.h"

//펄린노이즈 테스트용 지형
Terrain::Terrain()
{
	ReadHeightMap(Contents + L"HeightMaps/HeightMap256.dds"); //원본 높이맵
	CalcNormalVector();
	CalcTangentVector();
	AlignVertexData();

	D3DXMatrixIdentity(&world);
	dragIndices.clear();

}

Terrain::~Terrain()
{
}

void Terrain::CopyVertices(void * vertices)
{
	memcpy(vertices, this->vertices, sizeof(VertexTextureNormalTangent) * vertexCount);
}

void Terrain::ReadHeightMap(wstring file)
{
	vector<D3DXCOLOR> heights;

	Texture * heightTexture = new Texture(file);
	heightTexture->ReadPixels(DXGI_FORMAT_R8G8B8A8_UNORM, &heights);

	width = heightTexture->GetWidth() - 1;
	height = heightTexture->GetHeight() - 1;

	//Create VertexData
	{
		vertexCount = static_cast<UINT>((width + 1) * (height + 1));
		vertices = new VertexTextureNormalTangent[vertexCount];
		
		for (UINT z = 0; z <= height; z++)
		{
			for (UINT x = 0; x <= width; x++)
			{
				UINT index = (width + 1) * z + x;

				vertices[index].Position.x = (float)x;
				//높이맵 y값
				vertices[index].Position.y = (float)(heights[index].r * 255.0f) / 50.0f;
				vertices[index].Position.z = (float)z;

				vertices[index].Uv.x = (float)x / width;
				vertices[index].Uv.y = (float)z / height;
			}
		}//for(z)
	}

	//CreateIndexData
	{
		indexCount = static_cast<UINT>(width * height * 6);
		indices = new UINT[indexCount];

		UINT index = 0;
		for (UINT z = 0; z < height; z++)
		{
			for (UINT x = 0; x < width; x++)
			{
				indices[index + 0] = static_cast<UINT>((width + 1) * z + x); //0
				indices[index + 1] = static_cast<UINT>((width + 1) * (z + 1) + x); //1
				indices[index + 2] = static_cast<UINT>((width + 1) * z + (x + 1)); //2

				indices[index + 3] = static_cast<UINT>((width + 1) * z + (x + 1)); //2
				indices[index + 4] = static_cast<UINT>((width + 1) * (z + 1) + x); //1
				indices[index + 5] = static_cast<UINT>((width + 1) * (z + 1) + (x + 1)); //3

				index += 6;
			}
		}//for(z)
	}
}

void Terrain::CalcNormalVector()
{
	for (UINT i = 0; i < indexCount / 3; i++)
	{
		UINT index0 = indices[i * 3 + 0];
		UINT index1 = indices[i * 3 + 1];
		UINT index2 = indices[i * 3 + 2];

		VertexTextureNormalTangent v0 = vertices[index0];
		VertexTextureNormalTangent v1 = vertices[index1];
		VertexTextureNormalTangent v2 = vertices[index2];

		D3DXVECTOR3 d1 = v1.Position - v0.Position; //(0, 0, 1)
		D3DXVECTOR3 d2 = v2.Position - v0.Position; //(1, 0, 0)
		D3DXVECTOR3 normal;
		D3DXVec3Cross(&normal, &d1, &d2);

		vertices[index0].Normal += normal;
		vertices[index1].Normal += normal;
		vertices[index2].Normal += normal;
	}

	for (UINT i = 0; i < vertexCount; i++)
		D3DXVec3Normalize(&vertices[i].Normal, &vertices[i].Normal);
}

void Terrain::CalcTangentVector()
{	
	//노멀값 추가
	for (UINT i = 0; i < indexCount / 3; i++)
	{
		UINT index0 = indices[i * 3 + 0];
		UINT index1 = indices[i * 3 + 1];
		UINT index2 = indices[i * 3 + 2];

		VertexTextureNormalTangent vertex0 = vertices[index0];
		VertexTextureNormalTangent vertex1 = vertices[index1];
		VertexTextureNormalTangent vertex2 = vertices[index2];

		D3DXVECTOR3 e0 = vertex1.Position - vertex0.Position; 
		D3DXVECTOR3 e1 = vertex2.Position - vertex0.Position; 
		//정규직교
		float u0 = vertex1.Uv.x - vertex0.Uv.x; // 0
		float u1 = vertex2.Uv.x - vertex0.Uv.x; // 1/256
		float v0 = vertex1.Uv.y - vertex0.Uv.y; // 1/256
		float v1 = vertex2.Uv.y - vertex0.Uv.y; // 0
		float r = 1.0f / (u0 * v1 + v0 * u1);

		D3DXVECTOR3 tangent;
		tangent.x = (v1 * e0.x - v0 * e1.x);
		tangent.y = (v1 * e0.y - v0 * e1.y);
		tangent.z = (v1 * e0.z - v0 * e1.z);

		vertices[index0].Tangent += tangent;
		vertices[index1].Tangent += tangent;
		vertices[index2].Tangent += tangent;
	}
	
	for (UINT i = 0; i < vertexCount; i++)
	{
		D3DXVECTOR3 n = vertices[i].Normal;
		D3DXVECTOR3 t = vertices[i].Tangent;
	
		D3DXVECTOR3 temp = (t - n * D3DXVec3Dot(&n, &t));
		D3DXVec3Normalize(&temp, &temp);
	
		vertices[i].Tangent = temp;
	}
}

//정점 정렬
void Terrain::AlignVertexData()
{
	VertexTextureNormalTangent* vertexData = new VertexTextureNormalTangent[indexCount];

	UINT index = 0;
	for (UINT z = 0; z < height; z++)
	{
		for (UINT x = 0; x < width; x++)
		{
			UINT index1 = (width + 1) * z + x; //0
			UINT index2 = (width + 1) * z + (x + 1); //2
			UINT index3 = (width + 1) * (z + 1) + x;  //1
			UINT index4 = (width + 1) * (z + 1) + (x + 1);  //3

			vertexData[index] = vertices[index1]; index++; //0
			vertexData[index] = vertices[index3]; index++; //1
			vertexData[index] = vertices[index2]; index++; //2
			vertexData[index] = vertices[index2]; index++; //2
			vertexData[index] = vertices[index3]; index++; //1
			vertexData[index] = vertices[index4]; index++; //3
		}
	}

	SAFE_DELETE_ARRAY(vertices);

	vertexCount = indexCount;
	vertices = new VertexTextureNormalTangent[indexCount];
	memcpy(vertices, vertexData, sizeof(VertexTextureNormalTangent) * vertexCount);

	SAFE_DELETE_ARRAY(vertexData);
	SAFE_DELETE_ARRAY(indices);
}

bool Terrain::TerrainPicked(OUT D3DXVECTOR3 * out, Ray & ray)
{
	Context::Get()->GetMainCamera()->Position(&start);

	Context::Get()->GetMainCamera()->Matrix(&view);
	Context::Get()->GetPerspective()->GetMatrix(&projection);
	//World공간까지 역변환된 3차원 마우스 좌표
	Context::Get()->GetViewport()->GetRay(&ray, start, world, view, projection);

	direction = ray.Direction;

	for (UINT z = 0; z < height; z++)
	{
		for (UINT x = 0; x < width; x++)
		{
			int i = x * 6;
			UINT index[4];
			index[0] = static_cast<UINT>((((width * 6) * z)) + i);
			index[1] = static_cast<UINT>((((width * 6) * z)) + i + 1);
			index[2] = static_cast<UINT>((((width * 6) * z)) + i + 2);
			index[3] = static_cast<UINT>((((width * 6) * z)) + i + 5);

			D3DXVECTOR3 p[4];
			for (int i = 0; i < 4; i++)
				p[i] = vertices[index[i]].Position;

			float u, v;
			if (D3DXIntersectTri(&p[0], &p[1], &p[2], &ray.Position, &direction, &u, &v, &ray.Distance))
			{
				*out = p[0] + (p[1] - p[0]) * u + (p[2] - p[0]) * v;
				
				return true;
			}

			if (D3DXIntersectTri(&p[3], &p[1], &p[2], &ray.Position, &direction, &u, &v, &ray.Distance))
			{
				*out = p[3] + (p[1] - p[3]) * u + (p[2] - p[3]) * v;
			
				return true;
			}
		}
	}

	return false;
}