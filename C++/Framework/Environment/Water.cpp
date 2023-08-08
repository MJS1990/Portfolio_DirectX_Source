#include "Framework.h"
#include "Water.h"
#include "Viewer/Frustum.h"

const UINT Water::DivideCount = 1024;
//vertexCount, Culling�� ���� �ʱ�ȭ, ����Ʈ�� �Լ� ȣ��
Water::Water(UINT width, UINT height, float thick, Texture * heightMap, Frustum* frustum)
	: width(width), height(height), thick(thick)
	, heightMap(heightMap), vertices(NULL), indices(NULL)
	, frustum(frustum)
{
}

void Water::Initialize()
{
	vertexCount = (width + 1) * (height + 1);
	vertices = new VertexTexture[vertexCount];

	parent = new NodeType();

	shader = new Shader(Shaders + L"Water.fx");
	material = new Material(shader->GetFile());

	material->SetAmbient(0.25f, 0.20f, 1.0f, 1.0f);
	material->SetDiffuse(0.46f, 0.71f, 0.77f, 0.627f);
	material->SetSpecular(1.0f, 1.0f, 1.0f, 1.0f);
	material->SetNormalMap(Textures + L"WaveNormal.dds");
	material->SetShininess(500);

	//Create VertexData
	{
		for (UINT z = 0; z <= height; z++)
		{
			for (UINT x = 0; x <= width; x++)
			{
				UINT index = (width + 1) * z + x;

				vertices[index].Position.x = (float)x * thick;// -128.0f;
				vertices[index].Position.y = (float)0.0f;
				vertices[index].Position.z = (float)z * thick;// -128.0f;

				vertices[index].Uv.x = ((float)x / (float)width) * 2;
				vertices[index].Uv.y = ((float)z / (float)height) * 2;
			}
		}
	}

	//Create IndexData
	{
		indexCount = width * height * 6;
		indices = new UINT[indexCount];

		UINT index = 0;
		for (UINT z = 0; z < height; z++)
		{
			for (UINT x = 0; x < width; x++)
			{
				indices[index + 0] = (width + 1) * z + x; //0
				indices[index + 1] = (width + 1) * (z + 1) + x; //1
				indices[index + 2] = (width + 1) * z + (x + 1); //2

				indices[index + 3] = (width + 1) * z + (x + 1); //2
				indices[index + 4] = (width + 1) * (z + 1) + x; //1
				indices[index + 5] = (width + 1) * (z + 1) + (x + 1); //3

				index += 6;
			}
		}
	}

	AlignVertexData();

	triangleCount = vertexCount / 3;

	float centerX = 0.0f, centerZ = 0.0f, quadWidth = 0.0f;
	//�߾����� ������ �ִ�ʺ� ���
	CalcMeshDimensions(vertexCount, centerX, centerZ, quadWidth);

	CreateTreeNode(parent, centerX, centerZ, quadWidth);
	//�⺻frustum ����
	if (this->frustum == NULL)
	{
		D3DDesc desc;
		D3D::GetDesc(&desc);
		perspectiveForCulling = new Perspective(desc.Width, desc.Height, (float)D3DX_PI * 0.7f, 0.1f, 1000.0f);
		this->frustum = new Frustum(1000, Context::Get()->GetMainCamera(), perspectiveForCulling);
	}
	//

	vsBufferData = new VsBufferData();
	psBufferData = new PsBufferData();

	oceanSize = D3DXVECTOR2(width * thick, height * thick);
	psBufferData->OceanSize = oceanSize;

	//Bind CBuffer
	vsBuffer = new CBuffer(material->GetShader(), "VS_Ocean", vsBufferData, sizeof(VsBufferData));
	psBuffer = new CBuffer(material->GetShader(), "PS_Ocean", psBufferData, sizeof(PsBufferData));
	//
	shader->AsSRV("HeightMap")->SetResource(heightMap->SRV());
}

void Water::Ready()
{
	//Create VertexData
	{
		for (UINT z = 0; z <= height; z++)
		{
			for (UINT x = 0; x <= width; x++)
			{
				UINT index = (width + 1) * z + x;

				vertices[index].Position.x = (float)x * thick;// -128.0f;
				vertices[index].Position.y = (float)0.0f;
				vertices[index].Position.z = (float)z * thick;// -128.0f;

				vertices[index].Uv.x = ((float)x / (float)width) * 2;
				vertices[index].Uv.y = ((float)z / (float)height) * 2;
			}
		}
	}

	//Create IndexData
	{
		indexCount = width * height * 6;
		indices = new UINT[indexCount];

		UINT index = 0;
		for (UINT z = 0; z < height; z++)
		{
			for (UINT x = 0; x < width; x++)
			{
				indices[index + 0] = (width + 1) * z + x; //0
				indices[index + 1] = (width + 1) * (z + 1) + x; //1
				indices[index + 2] = (width + 1) * z + (x + 1); //2
				
				indices[index + 3] = (width + 1) * z + (x + 1); //2
				indices[index + 4] = (width + 1) * (z + 1) + x; //1
				indices[index + 5] = (width + 1) * (z + 1) + (x + 1); //3

				index += 6;
			}
		}
	}

	//Create VertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTexture) * vertexCount;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	
		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;
	
		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}
	
	//Create IndexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(UINT) * indexCount;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	
		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = indices;
	
		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &indexBuffer);
		assert(SUCCEEDED(hr));
	}
	/////////////////////////////////////////////////////////////////////////

	AlignVertexData();

	triangleCount = vertexCount / 3;

	float centerX = 0.0f, centerZ = 0.0f, quadWidth = 0.0f;
	//�߾����� ������ �ִ�ʺ� ���
	CalcMeshDimensions(vertexCount, centerX, centerZ, quadWidth);

	CreateTreeNode(parent, centerX, centerZ, quadWidth);
	//�⺻frustum ����
	if (this->frustum == NULL)
	{
		D3DDesc desc;
		D3D::GetDesc(&desc);
		perspectiveForCulling = new Perspective(desc.Width, desc.Height, (float)D3DX_PI * 0.7f, 0.1f, 1000.0f);
		this->frustum = new Frustum(1000, Context::Get()->GetMainCamera(), perspectiveForCulling);
	}
	//

	vsBufferData = new VsBufferData();
	psBufferData = new PsBufferData();
	
	oceanSize = D3DXVECTOR2(width * thick, height * thick);
	psBufferData->OceanSize = oceanSize;

	//Bind CBuffer
	vsBuffer = new CBuffer(material->GetShader(), "VS_Ocean", vsBufferData, sizeof(VsBufferData));
	psBuffer = new CBuffer(material->GetShader(), "PS_Ocean", psBufferData, sizeof(PsBufferData));
	//
	shader->AsSRV("HeightMap")->SetResource(heightMap->SRV());

	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);
}

Water::~Water()
{
	DeleteNode(parent);
	SAFE_DELETE(parent);
	SAFE_DELETE(frustum);

	SAFE_DELETE(material);
	SAFE_DELETE(perspectiveForCulling);

	SAFE_DELETE(shader);

	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);

	SAFE_DELETE(vsBufferData);
	SAFE_DELETE(psBufferData);

	SAFE_DELETE(normalTexture);
}

void Water::Update()
{
	frustum->Update();

	D3DXMATRIX T;
	//D3DXMatrixIdentity(&T);
	D3DXMatrixTranslation(&T, 0, 2.0f, 0);

	material->SetWorld(T);
	
	runningTime = Time::Get()->Running();
	vsBufferData->RunningTime = runningTime;
	vsBuffer->Change();
}

void Water::Render()
{
	//���� �� ����
	ImGui::Separator();

	ImGui::Text("OceanSettings");

	ImGui::SliderFloat("WaveFrequency", &vsBufferData->WaveFrequency, 0.0f, 1.0f);
	ImGui::SliderFloat("WaveAmplitude", &vsBufferData->WaveAmplitude, 0.0f, 50.0f);
	ImGui::SliderFloat2("TextureScale", vsBufferData->TextureScale, 0.0f, 50.0f);
	ImGui::SliderFloat2("BumpSpeed", vsBufferData->BumpSpeed, 0.0f, 15.0f);
	ImGui::SliderFloat("BumpHeight", &vsBufferData->BumpHeight, 0.0f, 15.0f);
	ImGui::SliderFloat("RunningTime", &vsBufferData->RunningTime, 0.0f, 150.0f);

	ImGui::Separator();

	drawCount = 0;
	RenderNode(parent);

	ImGui::Text("Ocean Draw : %d", drawCount);
	ImGui::Separator();
}

void Water::RenderNode(NodeType * node)
{
	D3DXVECTOR3 center(node->X, 0.0f, node->Z);
	float d = node->Width / 2.0f;
	
	D3DXVECTOR3 cameraPos;
	Context::Get()->GetMainCamera()->Position(&cameraPos);
	float zDistance = center.z - cameraPos.z;
	D3DXVECTOR3 distance = center - cameraPos;
	
	//����ü�� ������ �鸸 �׷���, �߰��� �ٰŸ��� �ø� ����
	if ((distance.x > 20.0f || distance.x < -20.0f) && distance.z > 20.0f)
	{
		if (frustum->ContainCube(center, d) == false)
			return;
	}
	UINT count = 0;
	for (int i = 0; i < 4; i++)
	{
		if (node->Childs[i] != NULL)
		{
			count++;

			RenderNode(node->Childs[i]); //�ڽĳ��� ���
		}
	}

	if (count != 0)
		return;

	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &node->VertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(node->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT indexCount = node->TriangleCount * 3;
	material->GetShader()->DrawIndexed(0, 0, indexCount);

	drawCount += node->TriangleCount;
}

void Water::DeleteNode(NodeType * node)
{
	for (int i = 0; i < 4; i++)
	{
		if(node->Childs[i] != NULL)
			DeleteNode(node->Childs[i]);

		SAFE_DELETE(node->Childs[i]);
	}

	SAFE_RELEASE(node->VertexBuffer);
	SAFE_RELEASE(node->IndexBuffer);
}

void Water::CalcMeshDimensions(UINT vertexCount, float & centerX, float & centerZ, float & meshWidth)
{
	centerX = centerZ = 0.0f;

	for (UINT i = 0; i < vertexCount; i++)
	{
		centerX += vertices[i].Position.x;
		centerZ += vertices[i].Position.z;
	}
	//���� �߾� ���
	centerX = centerX / (float)vertexCount;
	centerZ = centerZ / (float)vertexCount;

	float maxWidth = 0.0f;
	float maxDepth = 0.0f;

	//�������κ��� �߾ӱ����� ����, ����(���밪 ����)
	float minWidth = fabsf(vertices[0].Position.x - centerX);
	float minDepth = fabsf(vertices[0].Position.z - centerZ);

	for (UINT i = 0; i < vertexCount; i++) //centerX, Z�������� ��������, �� �ȿ��� �ִ�, �ּҳ���, ���� ã�� �κ�
	{
		float width = fabsf(vertices[i].Position.x - centerX);
		float depth = fabsf(vertices[i].Position.z - centerZ);

		if (width > maxWidth) maxWidth = width;
		if (depth > maxDepth) maxDepth = depth;
		if (width < minWidth) minWidth = width;
		if (depth < minDepth) minDepth = depth;
	}

	float maxX = (float)max(fabsf(minWidth), fabsf(maxWidth));
	float maxZ = (float)max(fabsf(minDepth), fabsf(maxDepth));


	meshWidth = (float)max(maxX, maxZ) * 2.0f; //�������� ������ �ִ� ����
}

void Water::CreateTreeNode(NodeType * node, float positionX, float positionZ, float width)
{
	node->X = positionX; //centerX
	node->Z = positionZ; //centerZ

	node->TriangleCount = 0;

	node->VertexBuffer = NULL;
	node->IndexBuffer = NULL;

	for (UINT i = 0; i < 4; i++)
		node->Childs[i] = NULL;
	///////////////

	//case1 ���� ������ ������
	//tirangles->�ڽĳ�尡 �����ϴ� �ﰢ�� ����
	//�����ϴ� �����κп� ���ϴ� �ﰢ�� ���� ����
	UINT triangles = ContainTriangleCount(positionX, positionZ, width);
	if (triangles == 0) //�����ϴ� �ﰢ���� ������ ����
		return;

	//case2 �� ���� ���� ����
	if (triangles > DivideCount)
	{
		for (UINT i = 0; i < 4; i++)
		{
			//centerX, Z �������� ���� 4�� ���� �� �߽���
			float offsetX = (((i % 2) < 1) ? -1.0f : 1.0f) * (width / 4.0f); //-1,  1, -1,  1
			float offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f) * (width / 4.0f); //-1, -1,  1,  1

			UINT count = ContainTriangleCount((positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));
			//���� �� �ȿ� �ﰢ���� �����ϸ� �ڽĳ�� ����
			//-> ��ͷ� �ﰢ���� ���� ��ȿ� �������� ���������� �ݺ��ؼ� Ʈ������
			if (count > 0)
			{
				node->Childs[i] = new NodeType();

				CreateTreeNode(node->Childs[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));
			}
		}

		return;
	}

	//case3 ���� ������ ������(triangles <= DivideCount), �������� �ϴ� ���� �ּ� �ﰢ�� �������� ������
	node->TriangleCount = triangles;

	//��� ������ ����� ��� ����
	UINT vertexCount = triangles * 3;

	VertexTexture* vertices = new VertexTexture[vertexCount];
	UINT* indices = new UINT[vertexCount];

	UINT index = 0, vertexIndex = 0;
	for (UINT i = 0; i < triangleCount; i++)
	{
		if (IsTriangleContained(i, positionX, positionZ, width) == true)
		{
			vertexIndex = i * 3;

			vertices[index].Position = this->vertices[vertexIndex].Position;
			vertices[index].Uv = this->vertices[vertexIndex].Uv;
			indices[index] = index;

			index++;
			vertexIndex++;

			vertices[index].Position = this->vertices[vertexIndex].Position;
			vertices[index].Uv = this->vertices[vertexIndex].Uv;
			indices[index] = index;

			index++;
			vertexIndex++;

			vertices[index].Position = this->vertices[vertexIndex].Position;
			vertices[index].Uv = this->vertices[vertexIndex].Uv;
			indices[index] = index;

			index++;
		}
	}

	//CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTexture) * vertexCount;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &(node->VertexBuffer));
		assert(SUCCEEDED(hr));
	}

	//CreateIndexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(UINT) * vertexCount;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = indices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &(node->IndexBuffer));
		assert(SUCCEEDED(hr));
	}

	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);
}

UINT Water::ContainTriangleCount(float positionX, float positionZ, float width)
{
	UINT count = 0;

	for (UINT i = 0; i < triangleCount; i++)
	{
		//�ﰢ���� ���ȿ� �����ִ��� üũ
		if (IsTriangleContained(i, positionX, positionZ, width) == true)
			count++;
	}

	return count;
}

bool Water::IsTriangleContained(UINT index, float positionX, float positionZ, float width)
{
	float radius = width / 2.0f;
	//index�� �ﰢ���� �ε����̹Ƿ� 3���ؼ� �ﰢ���� �̷�� ������ ���� �ε����� ��ȯ
	UINT vertexIndex = index * 3;
	//�ﰢ���� �� ���� ��ġ
	float x1 = vertices[vertexIndex].Position.x;
	float z1 = vertices[vertexIndex].Position.z;
	vertexIndex++;

	float x2 = vertices[vertexIndex].Position.x;
	float z2 = vertices[vertexIndex].Position.z;
	vertexIndex++;

	float x3 = vertices[vertexIndex].Position.x;
	float z3 = vertices[vertexIndex].Position.z;

	//�簢�� �߽ɰ� ���������� �簢�� �ȿ� �ﰢ���� ���� �ִ��� Ȯ��
	float minimumX = min(x1, min(x2, x3));
	if (minimumX > (positionX + radius))
		return false;

	float maximumX = max(x1, max(x2, x3));
	if (maximumX < (positionX - radius))
		return false;

	float minimumZ = min(z1, min(z2, z3));
	if (minimumZ >(positionZ + radius))
		return false;

	float maximumZ = max(z1, max(z2, z3));
	if (maximumZ < (positionZ - radius))
		return false;

	return true;
}


//���� ����
void Water::AlignVertexData()
{
	VertexTexture* vertexData = new VertexTexture[indexCount];

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
	vertices = new VertexTexture[indexCount];
	memcpy(vertices, vertexData, sizeof(VertexTexture) * vertexCount);

	SAFE_DELETE_ARRAY(vertexData);
	SAFE_DELETE_ARRAY(indices);
}