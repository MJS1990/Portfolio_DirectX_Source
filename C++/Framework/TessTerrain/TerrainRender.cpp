#include "Framework.h"
#include "TerrainRender.h"
#include "TessTerrain.h"
#include "TessHeightMap.h"
#include "Viewer/Frustum.h"

const float TerrainRender::MaxDistance = 500.0f;
const float TerrainRender::MinDistance = 20.0f;
const float TerrainRender::MaxTessellation = 6.0f;
const float TerrainRender::MinTessellation = 0.0f;
//TODO : ����(2048 * 2048) > ��(64(8 * 8), ���������ø���) > ��ġ() > ����(��ġ�ϳ��� 4��)
TerrainRender::TerrainRender(TessTerrain * terrain)
	: terrain(terrain)
{
}

TerrainRender::~TerrainRender()
{
	SAFE_DELETE(terrainDiffuseMap);
	SAFE_DELETE(terrainNormalMap);

	SAFE_RELEASE(quadPatchVB);
	SAFE_RELEASE(quadPatchIB);
}

void TerrainRender::Initialize()
{
	D3DXMatrixIdentity(&world);

	frustum = new Frustum(1000);

	//�� ��ĭ�� �� ���� ���ؽ� ����
	patchVertexRows = (((UINT)terrain->Desc().HeightMapHeight - 1) / TessTerrain::CellPerPatch) + 1; //�� ���� ������ �������� -> (UINT)(2048 / 65) + 1 -> 33
	patchVertexCols = (((UINT)terrain->Desc().HeightMapWidth - 1) / TessTerrain::CellPerPatch) + 1; //�� ���� ������ �������� -> (UINT)(2048 / 65) + 1 -> 33

	patchVerticesCount = patchVertexRows * patchVertexCols; //�� ���� �� �������� -> 1089
	patchQuadFacesCount = (patchVertexRows - 1) * (patchVertexCols - 1); //�� ���� �� �簢�� ���� -> 1024

	CalcAllPatchBoundsY();
}

void TerrainRender::Ready(Material * material)
{
	this->material = material;

	BuildQuadPatchVB(); //VertexBuffer����, ����
	BuildQuadPatchIB(); //IndexBuffer����, ����
	//cBuffer�� ���⿡�� ���� �� Render���� ������ ����, ���ε�
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(Buffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &cBuffer); //�� ���� ����� Render���� ������Ʈ
	assert(SUCCEEDED(hr));
	
	//�ؽ���
	wstring diffuseTexture = terrain->Desc().DiffuseMapFile;
	wstring normalTexture = terrain->Desc().NormalMapFile;
	
	terrainDiffuseMap = new Texture(diffuseTexture);
	terrainMapSRV = terrainDiffuseMap->SRV();

	terrainNormalMap = new Texture(normalTexture); //���⼭ ����
	normalMapSRV = terrainNormalMap->SRV();

	heightMapSRV = terrain->GetHeightMap()->BuildSRV();

	Shader* shader = material->GetShader();
	heightMapVariable = shader->AsSRV("HeightMap");
	terrainMapVariable = shader->AsSRV("TerrainMap");
	normalMapVariable = shader->AsSRV("NormalMap");

	heightMapVariable->SetResource(heightMapSRV);
	terrainMapVariable->SetResource(terrainMapSRV);
	normalMapVariable->SetResource(normalMapSRV);

	terrainVariable = material->GetShader()->AsConstantBuffer("CB_Terrain");
}

void TerrainRender::Render()
{
	buffer.FogStart = 300.0f;
	buffer.FogRange = 200.0f;
	buffer.FogColor = D3DXCOLOR(1, 1, 1, 1);
	buffer.MinDistance = MinDistance;
	buffer.MaxDistance = MaxDistance;
	buffer.MinTessellation = MinTessellation;
	buffer.MaxTessellation = MaxTessellation;
	buffer.TexelCellSpaceU = 1.0f / terrain->Desc().HeightMapWidth;
	buffer.TexelCellSpaceV = 1.0f / terrain->Desc().HeightMapHeight;
	buffer.WorldCellSpace = terrain->Desc().CellSpacing;
	
	frustum->GetPlanes(buffer.WorldFrustumPlanes);
	
	D3D11_MAPPED_SUBRESOURCE subResource;
	HRESULT hr = D3D::GetDC()->Map(cBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource); //TODO : ���⼭ ����
	assert(SUCCEEDED(hr));
	
	memcpy(subResource.pData, &buffer, sizeof(Buffer));
	D3D::GetDC()->Unmap(cBuffer, 0);
	
	terrainVariable->SetConstantBuffer(cBuffer);
	assert(SUCCEEDED(hr));

	UINT stride = sizeof(TerrainCP);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &quadPatchVB, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(quadPatchIB, DXGI_FORMAT_R16_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	material->SetWorld(world);
	material->GetShader()->DrawIndexed(0, 0, patchQuadFacesCount * 4);
}

void TerrainRender::CalcAllPatchBoundsY()
{
	patchBoundsY.assign(patchQuadFacesCount, D3DXVECTOR2()); //�� ��ĭ�� �簢��������ŭ �����Ҵ� 
	//for(row -> 0 ~ 32),for(col -> 0 ~ 32)
	for (UINT row = 0; row < patchVertexRows - 1; row++) //�� ���� ��?�� �簢�� ����(for(0~31))
	{
		for (UINT col = 0; col < patchVertexCols - 1; col++) //�� ���� ��?�� �簢�� ����(for((0~31))
			CalcPatchBounds(row, col);
	}
}
//�ش� ���� ����(row, col)�ȿ��� ���� ���� ����(Texture�� r��), ���� ū ���̰� ���Ϳ� ����
//�Ķ���Ϳ� �簢���� �� ����, ����ĭ�� �ε����� ��
void TerrainRender::CalcPatchBounds(UINT row, UINT col) //�� ��ġ���� �˻� -> �ش� ��ġ�� ���� ���� ����(���� �ϴ�)�� �ε����� ���� ū ����(���� ���)�� �ε����� ����
{
	UINT x0 = col * TessTerrain::CellPerPatch; //CellPerPatch->64, 0 ~ 1984
	UINT x1 = (col + 1) * TessTerrain::CellPerPatch; //1 ~ 2048

	UINT y0 = row * TessTerrain::CellPerPatch; //0 ~ 1984
	UINT y1 = (row + 1) * TessTerrain::CellPerPatch; //1 ~ 2048

	float minY = FLT_MAX;
	float maxY = FLT_MIN;

	for (UINT y = y0; y <= y1; y++) //0~64, 64~128 -> 64�� �ݺ�
	{
		for (UINT x = x0; x <= x1; x++) //0~64, 64~128 -> 64�� �ݺ�
		{
			minY = min(minY, terrain->Data(y, x)); //Data->�ش� ��ġ�� �ؽ��İ�(r->���̰�) ��ȯ
			maxY = max(maxY, terrain->Data(y, x));
		}
	}

	UINT patchID = row * (patchVertexCols - 1) + col; //�� ������ �ش� row, col ��ġ ������ �ε���?, 0 ~ 1056
	patchBoundsY[patchID] = D3DXVECTOR2(minY, maxY); //�ش� row, col���� ���ȿ��� �ּ�, �ִ� ���̰� ����
}

void TerrainRender::BuildQuadPatchVB()
{
	TerrainCP* patchVertices = new TerrainCP[patchVerticesCount];
	float halfWidth = 0.5f * (float)terrain->Width(); //0.5 * 1024 = 512
	float halfDepth = 0.5f * (float)terrain->Depth(); //0.5 * 1024 = 512

	float patchWidth = (float)terrain->Width() / (float)(patchVertexCols - 1); //�������� �ʺ� -> 1024 / 32 = 32
	float patchDepth = (float)terrain->Depth() / (float)(patchVertexRows - 1); //�������� ���� -> 1024 / 32 = 32

	float du = 1.0f / (float)(patchVertexCols - 1); //�� ��ġ���� uv.x, 1 / 32
	float dv = 1.0f / (float)(patchVertexRows - 1); //�� ��ġ���� uv.y, 1 / 32

	//TODO : ������ �������� ������ ����
	//33X33���� �̷���� �� ��ġ�� ���������� ���� �� ����
	for (UINT row = 0; row < patchVertexRows; row++) //0 ~ 32
	{
		float z = halfDepth - (float)row * patchDepth; //512 ~ -512(-16)
		for (UINT col = 0; col < patchVertexCols; col++) //0 ~ 32
		{
			float x = -halfWidth + (float)col * patchWidth; //-512 ~ 512(+16)
			UINT vertId = row * patchVertexCols + col; //TODO : 0 ~ 1089
			
			patchVertices[vertId].Position = D3DXVECTOR3(x, 0, z);
			patchVertices[vertId].Uv = D3DXVECTOR2(col * du, row * dv);
		}
	}
	//���μ��� �׸��°�
	for (UINT row = 0; row < patchVertexRows - 1; row++) //0 ~ 31
	{
		for (UINT col = 0; col < patchVertexCols - 1; col++) //0 ~ 31
		{
			UINT patchID = row * (patchVertexCols - 1) + col; //0~1023
			UINT vertID = row * patchVertexCols + col; //0~1054
			patchVertices[vertID].BoundsY = patchBoundsY[patchID]; //�ش� ������ ���� ���� �ּ�(�����ϴ�), �ִ�(�������)���� ��ġ��
		}
	}
	//TODO : patchVertices ���� ���� CalcNormalVector, CalcTangentVector����
	CalcNormalVector(patchVertices);
	CalcTangentVector(patchVertices);
	//

	D3D11_BUFFER_DESC desc = { 0 };
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = sizeof(TerrainCP) * patchVerticesCount;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = &patchVertices[0];

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &quadPatchVB);
	assert(SUCCEEDED(hr));

	SAFE_DELETE_ARRAY(patchVertices);
}

void TerrainRender::BuildQuadPatchIB()
{
	vector<WORD> indices;
	for (WORD row = 0; row < (WORD)patchVertexRows - 1; row++) //0~31
	{
		for (WORD col = 0; col < (WORD)patchVertexCols - 1; col++) //0~31
		{
			indices.push_back(row * (WORD)patchVertexCols + col); //���� �Ʒ�
			indices.push_back(row * (WORD)patchVertexCols + col + 1); //������ �Ʒ�
			indices.push_back((row + 1) * (WORD)patchVertexCols + col); //���� ��
			indices.push_back((row + 1) * (WORD)patchVertexCols + col + 1); //������ ��
		}
	}

	D3D11_BUFFER_DESC desc = { 0 };
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = sizeof(WORD) * indices.size();
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = &indices[0];

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &quadPatchIB);
	assert(SUCCEEDED(hr));
}

void TerrainRender::CalcNormalVector(TerrainCP * pVertices)
{
	//UINT triCount = ((patchVertexRows - 1) * (patchVertexCols - 1)) * 2;
	//TODO : �ﰢ�� ������ �ݺ��ؾ� ��
	//��ְ� �߰�
	for (UINT i = 0; i < patchVerticesCount - 32; i++)
	{
		UINT index0 = 0;
		UINT index1 = 0;
		UINT index2 = 0;
		if (i % 2 == 0)
		{
			index0 = i;
			index1 = patchVertexRows + i; //33 + i
			index2 = i + 1;
		}
		else
		{
			index0 = (i / 2) + 1;
			index1 = index0 + 31;
			index2 = index1 + 1;
		}

		TerrainCP v0 = pVertices[index0];
		TerrainCP v1 = pVertices[index1];
		TerrainCP v2 = pVertices[index2];

		D3DXVECTOR3 d1 = v1.Position - v0.Position;
		D3DXVECTOR3 d2 = v2.Position - v0.Position;

		D3DXVECTOR3 normal;
		D3DXVec3Cross(&normal, &d1, &d2);

		pVertices[index0].Normal += normal;
		pVertices[index1].Normal += normal;
		pVertices[index2].Normal += normal;
	}
	//TODO : pVertices�� ��ְ� ����� ���� ���� �� ������ ��ְ��� ������ ������ �ΰ��� ��ְ� ���������� �����Ұ�

	for (UINT i = 0; i < patchVerticesCount; i++)
		D3DXVec3Normalize(&pVertices[i].Normal, &pVertices[i].Normal);
	
}

void TerrainRender::CalcTangentVector(TerrainCP* pVertices)
{
	//UINT triCount = ((patchVertexRows - 1) * (patchVertexCols - 1));
	//TODO : �ﰢ�� ������ �ݺ��ؾ� ��
	//��ְ� �߰�
	for (UINT i = 0; i < patchVerticesCount - 32; i++)
	{
		UINT index0 = 0;
		UINT index1 = 0;
		UINT index2 = 0;
		if (i % 2 == 0)
		{
			index0 = i;
			index1 = patchVertexRows + i;
			index2 = i + 1;
		}
		else
		{
			index0 = (i / 2) + 1;
			index1 = index0 + 31;
			index2 = index1 + 1;
		}
		
		TerrainCP vertex0 = pVertices[index0]; //0, 2
		TerrainCP vertex1 = pVertices[index1]; //1, 1
		TerrainCP vertex2 = pVertices[index2]; //2, 3
	
		D3DXVECTOR3 e0 = vertex1.Position - vertex0.Position;
		D3DXVECTOR3 e1 = vertex2.Position - vertex0.Position;
		//��������
		float u0 = vertex1.Uv.x - vertex0.Uv.x; // 0
		float u1 = vertex2.Uv.x - vertex0.Uv.x; // 1/256
		float v0 = vertex1.Uv.y - vertex0.Uv.y; // 1/256
		float v1 = vertex2.Uv.y - vertex0.Uv.y; // 0
		float r = 1.0f / (u0 * v1 + v0 * u1);
	
		D3DXVECTOR3 tangent;
		tangent.x = (v1 * e0.x - v0 * e1.x);
		tangent.y = (v1 * e0.y - v0 * e1.y);
		tangent.z = (v1 * e0.z - v0 * e1.z);
		//tangent.x = r * (v1 * e0.x - v0 * e1.x);
		//tangent.y = r * (v1 * e0.y - v0 * e1.y);
		//tangent.z = r * (v1 * e0.z - v0 * e1.z);
	
		pVertices[index0].Tangent += tangent;
		pVertices[index1].Tangent += tangent;
		pVertices[index2].Tangent += tangent;
	}
	
	for (UINT i = 0; i < patchVerticesCount; i++)
	{
		D3DXVECTOR3 n = pVertices[i].Normal;
		D3DXVECTOR3 t = pVertices[i].Tangent;
		
		D3DXVECTOR3 temp = (t - n * D3DXVec3Dot(&n, &t));
		D3DXVec3Normalize(&temp, &temp);
	
		pVertices[i].Tangent = temp;
	}
}
