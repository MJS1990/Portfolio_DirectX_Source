#include "Framework.h"
#include "MeshHemiSphere.h"

MeshHemiSphere::MeshHemiSphere(Material* material, UINT drawCount)
	: Mesh(material), drawCount(drawCount)
{
}

MeshHemiSphere::~MeshHemiSphere()
{
}

void MeshHemiSphere::CreateData()
{
	UINT latitude = drawCount / 2; //위도 (남<->북)
	UINT longitude = drawCount; //경도 (서<->동)

	vertexCount = longitude * latitude * 2;
	indexCount = (longitude - 1) * (latitude - 1) * 2 * 8;

	vertices = new VertexTextureNormalTangent[vertexCount]; 

	UINT index = 0;
	for (UINT i = 0; i < longitude; i++)
	{
		float xz = 100.0f * (i / (longitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = Math::PI * j / (latitude - 1);

			vertices[index].Position.x = sinf(xz) * cosf(y);
			vertices[index].Position.y = cosf(xz);
			vertices[index].Position.z = sinf(xz) * sinf(y);
			vertices[index].Position *= 10.0f;

			vertices[index].Uv.x = 0.5f / (float)longitude + i / (float)longitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + i / (float)latitude;

			index++;
		}
	}

	for (UINT i = 0; i < longitude; i++)
	{
		float xz = 100.0f * (i / (longitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = (Math::PI * 2.0f) - (Math::PI * j) / (latitude - 1);

			vertices[index].Position.x = sinf(xz) * cosf(y);
			vertices[index].Position.y = cosf(xz);
			vertices[index].Position.z = sinf(xz) * sinf(y);
			vertices[index].Position *= 10.0f;

			vertices[index].Uv.x = 0.5f / (float)longitude + i / (float)longitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + i / (float)latitude;

			index++;
		}
	}

	index = 0;
	//UINT* indices = new UINT[indexCount];
	indices = new UINT[indexCount];
	for (UINT i = 0; i < longitude - 1; i++)
	{
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = i * latitude + j;
			indices[index++] = (i + 1) * latitude + j;
			indices[index++] = (i + 1) * latitude + j + 1;

			indices[index++] = (i + 1) * latitude + j + 1;
			indices[index++] = i * latitude + j + 1;
			indices[index++] = i * latitude + j;
		}
	}

	UINT offset = latitude * longitude;
	for (UINT i = 0; i < longitude - 1; i++)
	{
		for (UINT j = 0; j < longitude - 1; j++)
		{
			indices[index++] = offset + i * latitude + j;
			indices[index++] = offset + (i + 1) * latitude + j + 1;
			indices[index++] = offset + (i + 1) * latitude + j;

			indices[index++] = offset + i * latitude + j + 1;
			indices[index++] = offset + (i + 1) * latitude + j + 1;
			indices[index++] = offset + i * latitude + j;
		}
	}
}
