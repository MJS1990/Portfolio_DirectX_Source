#include "Framework.h"
#include "QuadTree.h"
#include "Environment/Terrain.h"
#include "Viewer/Frustum.h"
#include "Editor/Brush.h"

//const UINT QuadTree::DivideCount = 512;
const UINT QuadTree::DivideCount = 1024; //1024, 32x32

//Terrain클래스로부터 지형의 정점 정보를 가져와 쿼드트리를 사용해 나눈 후 
//프러스텀의 시야 안의 지형만 이 클래스에서 렌더링 수행
QuadTree::QuadTree(Terrain* terrain, Brush* brush, Frustum* frustum)
	: terrain(terrain), brush(brush), frustum(frustum), drawCount(0)
{
}

QuadTree::~QuadTree()
{
	DeleteNode(parent);
	SAFE_DELETE(parent);
	SAFE_DELETE(frustum);

	SAFE_DELETE(csData);

	SAFE_DELETE(material);
	
	SAFE_DELETE(perspectiveForCulling);

	SAFE_DELETE(csHeightTex);
}

void QuadTree::Initialize()
{
	material = new Material(Shaders + L"QuadTree_Noise_ver2.fx"); //노이즈 테스트용
	//material->SetDiffuseMap(Textures + L"DirtH.dds"); //원본텍스쳐

	material->SetDiffuseMap(Textures + L"ground5_Diffuse.dds");
	material->SetNormalMap(Textures + L"ground5_Normal.tga");

	vertexCount = terrain->VertexCount();
	triangleCount = vertexCount / 3;

	vertices = new VertexTextureNormalTangent[vertexCount];
	terrain->CopyVertices((void *)vertices);

	centerX = 0.0f, centerZ = 0.0f, width = 0.0f;

	//중앙점과 지형의 최대 너비 계산
	CalcMeshDimensions(vertexCount, centerX, centerZ, width);

	parent = new NodeType();
	CreateTreeNode(parent, centerX, centerZ, width);

	csData = new CSData();

	//추가 - 컬링용 절두체 크기 변경
	if (this->frustum == NULL)
	{
		D3DDesc desc;
		D3D::GetDesc(&desc);
		//perspectiveForCulling = new Perspective(desc.Width, desc.Height, (float)D3DX_PI, 0.01f, 1000.0f); //(float)D3DX_PI * 0.7f
		perspectiveForCulling = new Perspective(desc.Width, desc.Height, (float)D3DX_PI * 0.6f);
		this->frustum = new Frustum(1000, Context::Get()->GetMainCamera(), perspectiveForCulling); //Context::Get()->GetPerspective()
	}

	InitCSDatas(vertices); //CSTexture생성

	SAFE_DELETE_ARRAY(vertices);

	D3DXMatrixIdentity(&world);

}

void QuadTree::Ready()
{
}

void QuadTree::Update()
{
	frustum->Update();
}

void QuadTree::Render()
{
	drawCount = 0;
	RenderNode(parent);

	ImGui::Text("Terrain Draw : %d", drawCount);
}

void QuadTree::RenderNode(NodeType * node)
{
	D3DXVECTOR3 center(node->X, 0.0f, node->Z);
	//추가
	D3DXVECTOR3 cameraPos;
	Context::Get()->GetMainCamera()->Position(&cameraPos);
	float zDistance = center.z - cameraPos.z;
	D3DXVECTOR3 distance = center - cameraPos;
	//

	float d = node->Width / 2.0f;
	//절두체에 들어오는 면만 그려냄, 추가로 근거리는 컬링 제외
	//if (zDistance > 20.0f)
	if((distance.x > 20.0f || distance.x < -20.0f) && distance.z > 20.0f)
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

			RenderNode(node->Childs[i]); //자식 노드 재귀 
		}
	}
	//잎 노드까지 반복후 잎 노드에서 버퍼 셋 & 랜더
	if (count != 0)
		return;
	
	////TODO : ComputeShader테스트후 수정필요하면 수정
	//if (brush->GetbUseBrush() == true)
	//{
	//	InitCSDatas(node); //node->Vertices.size = 1536
	//	
	//	material->GetShader()->AsSRV("CSInput")->SetResource(csSrv);
	//	material->GetShader()->AsUAV("CSOutput")->SetUnorderedAccessView(csUav);
	//	
	//	material->GetShader()->Dispatch(0, 0, 64, 64, 1);
	//	
	//	CopyCSOutput(node);
	//}
	////////////////////////////////////////////////

	////TODO : 브러쉬 수정버전 -> 브러쉬 다시볼때 여기가 CS실행코드
	//if (brush->GetbUseBrush() == true)
	//{	
	//	material->GetShader()->Dispatch(0, 0, 32, 32, 1);
	//}

	UINT stride = sizeof(VertexTextureNormalTangent);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &node->VertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(node->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	material->SetWorld(world);

	UINT indexCount = node->TriangleCount * 3;
	material->GetShader()->DrawIndexed(0, 0, indexCount);

	drawCount += node->TriangleCount;
}

void QuadTree::DeleteNode(NodeType * node)
{
	for (int i = 0; i < 4; i++)
	{
		if (node->Childs[i] != NULL)
			DeleteNode(node->Childs[i]);
		
		//SAFE_DELETE가 NULL체크를 하기때문에 if문으로 자식노드 NULL체크 할필요 없음
		SAFE_DELETE(node->Childs[i]); 
	}

	SAFE_RELEASE(node->VertexBuffer);
	SAFE_RELEASE(node->IndexBuffer);
}

//자식 노드 나누는 함수
void QuadTree::CalcMeshDimensions(UINT vertexCount, float & centerX, float & centerZ, float & meshWidth)
{
	centerX = centerZ = 0.0f;

	for (UINT i = 0; i < vertexCount; i++)
	{
		centerX += vertices[i].Position.x;
		centerZ += vertices[i].Position.z;
	}
	//지형 중앙 계산
	centerX = centerX / (float)vertexCount;
	centerZ = centerZ / (float)vertexCount;

	float maxWidth = 0.0f;
	float maxDepth = 0.0f;

	//원점으로부터 중앙까지의 넓이, 깊이(절대값 연산)
	float minWidth = fabsf(vertices[0].Position.x - centerX);
	float minDepth = fabsf(vertices[0].Position.z - centerZ);

	for (UINT i = 0; i < vertexCount; i++) //centerX, Z기준으로 나눴을때, 그 안에서 최대, 최소넓이, 깊이 찾는 부분
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
	//float maxX = (float)maxWidth;
	//float maxZ = (float)maxDepth;

	meshWidth = (float)max(maxX, maxZ) * 2.0f; //나누기전 지형(매쉬?)의 최대 넓이(or 길이)
}
//자식 노드 생성, positionX, Z에는 각 분할 지형의 중앙위치값이 들어가고 
//width에는 각 분할지형의 넓이가 들어간다, 재귀로 트리 생성
void QuadTree::CreateTreeNode(NodeType * node, float positionX, float positionZ, float width)
{
	//멤버 초기화//
	node->X = positionX; //centerX
	node->Z = positionZ; //centerZ
	
	node->TriangleCount = 0;

	node->VertexBuffer = NULL;
	node->IndexBuffer = NULL;

	for (UINT i = 0; i < 4; i++)
		node->Childs[i] = NULL;
	///////////////

	//case1 남은 갯수가 없을때
	//tirangles -> 자식노드가 포함하는 삼각형 갯수
	//참조하는 지형부분에 속하는 삼각형 갯수 저장
	UINT triangles = ContainTriangleCount(positionX, positionZ, width);
	if (triangles == 0) //포함하는 삼각형이 없으면 리턴
		return;

	//case2 더 작은 노드로 분할
	if (triangles > DivideCount)
	{
		for (UINT i = 0; i < 4; i++)
		{
			//centerX, Z 기준으로 나눈 4개 면의 각 중심점
			float offsetX = (((i % 2) < 1) ? -1.0f : 1.0f) * (width / 4.0f); //-1,  1, -1,  1
			float offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f) * (width / 4.0f); //-1, -1,  1,  1

			UINT count = ContainTriangleCount((positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));
			//나눈 면 안에 삼각형이 존재하면 자식노드 생성
			//-> 재귀로 삼각형이 나눈 면안에 존재하지 않을때까지 반복해서 트리생성
			if (count > 0) 
			{
				node->Childs[i] = new NodeType();

				CreateTreeNode(node->Childs[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));
			}
		}

		return;
	}

	//case3 남은 갯수가 없을때(triangles <= DivideCount), 나누고자 하는 면의 최소 삼각형 갯수보다 적을때
	node->TriangleCount = triangles;

	//노드 정보를 사용해 노드 생성
	UINT vertexCount = triangles * 3;
	
	VertexTextureNormalTangent* vertices = new VertexTextureNormalTangent[vertexCount];
	UINT* indices = new UINT[vertexCount];

	UINT index = 0, vertexIndex = 0;
	for (UINT i = 0; i < triangleCount; i++)
	{
		//포함되어있는지 다시 확인
		if (IsTriangleContained(i, positionX, positionZ, width) == true)
		{
			vertexIndex = i * 3;

			vertices[index].Position = this->vertices[vertexIndex].Position;
			//TODO : ResulrVertex대입부분 추가
			node->Vertices.push_back(this->vertices[vertexIndex].Position);
			vertices[index].Uv = this->vertices[vertexIndex].Uv;
			vertices[index].Normal = this->vertices[vertexIndex].Normal;
			vertices[index].Tangent = this->vertices[vertexIndex].Tangent;
			indices[index] = index;

			index++;
			vertexIndex++;

			vertices[index].Position = this->vertices[vertexIndex].Position;
			node->Vertices.push_back(this->vertices[vertexIndex].Position);
			vertices[index].Uv = this->vertices[vertexIndex].Uv;
			vertices[index].Normal = this->vertices[vertexIndex].Normal;
			vertices[index].Tangent = this->vertices[vertexIndex].Tangent;
			indices[index] = index;

			index++;
			vertexIndex++;

			vertices[index].Position = this->vertices[vertexIndex].Position;
			node->Vertices.push_back(this->vertices[vertexIndex].Position);
			vertices[index].Uv = this->vertices[vertexIndex].Uv;
			vertices[index].Normal = this->vertices[vertexIndex].Normal;
			vertices[index].Tangent = this->vertices[vertexIndex].Tangent;
			indices[index] = index;

			index++;
		}
	}

	//CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT; // TODO : 동적버퍼 변경 확인 필요
		desc.ByteWidth = sizeof(VertexTextureNormalTangent) * vertexCount;
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

UINT QuadTree::ContainTriangleCount(float positionX, float positionZ, float width)
{
	UINT count = 0;

	for (int i = 0; i < triangleCount; i++)
	{
		//삼각형이 노드안에 들어와있는지 체크
		if (IsTriangleContained(i, positionX, positionZ, width) == true)
			count++;
	}

	return count;
}

//삼각형이 노드안에 들어와있는지 체크
bool QuadTree::IsTriangleContained(UINT index, float positionX, float positionZ, float width)
{
	float radius = width / 2.0f;
	//index는 삼각형의 인덱스이므로 3곱해서 삼각형을 이루는 정점의 인덱스로 변환
	UINT vertexIndex = index * 3; 
	//삼각형의 각 정점 위치
	float x1 = vertices[vertexIndex].Position.x;
	float z1 = vertices[vertexIndex].Position.z;
	vertexIndex++;

	float x2 = vertices[vertexIndex].Position.x;
	float z2 = vertices[vertexIndex].Position.z;
	vertexIndex++;

	float x3 = vertices[vertexIndex].Position.x;
	float z3 = vertices[vertexIndex].Position.z;

	//사각형 중심과 반지름으로 사각형 안에 삼각형이 들어와 있는지 확인
	float minimumX = min(x1, min(x2, x3));
	if (minimumX > (positionX + radius))  
		return false;
	
	float maximumX = max(x1, max(x2, x3));
	if (maximumX < (positionX - radius))
		return false;
	
	float minimumZ = min(z1, min(z2, z3));
	if (minimumZ > (positionZ + radius))
		return false;
	
	float maximumZ = max(z1, max(z2, z3));
	if (maximumZ < (positionZ - radius))
		return false;

	return true;
}

void QuadTree::InitCSDatas(VertexTextureNormalTangent* terrainData)
{	
	vector<D3DXVECTOR3> tPos;

	for (int i = 0; i < vertexCount; i++)
	{
		tPos.push_back(terrainData[i].Position);
	}

	HRESULT hr;

	//Bind TerrainVertices
	D3D11_BUFFER_DESC inputDesc;
	inputDesc.Usage = D3D11_USAGE_DEFAULT;
	inputDesc.ByteWidth = sizeof(D3DXVECTOR3) * vertexCount; //node->Vertices.size() * sizeof(D3DXVECTOR3);
	inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inputDesc.CPUAccessFlags = 0; //cpu에서 읽지 않으므로 0 
	inputDesc.StructureByteStride = sizeof(D3DXVECTOR3); //structeredBuffer하나의 크기
	inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; //버퍼용도

	D3D11_SUBRESOURCE_DATA data = { 0 };
	//data.pSysMem = (&terrainData[0])->Position;
	data.pSysMem = &tPos[0];

	ID3D11Buffer* buffer = NULL;
	hr = D3D::GetDevice()->CreateBuffer(&inputDesc, &data, &buffer);
	assert(SUCCEEDED(hr)); //TODO : 마우스 누른상태에서 에러
	
	{
		//뷰 생성
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		//structuredBuffer는 BUFFEREX로 설정
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.FirstElement = 0;
		srvDesc.BufferEx.Flags = 0;
		srvDesc.BufferEx.NumElements = vertexCount;

		hr = D3D::GetDevice()->CreateShaderResourceView(buffer, &srvDesc, &csSrv);
		assert(SUCCEEDED(hr));

		material->GetShader()->AsSRV("CSVertexData")->SetResource(csSrv);
	}

	//Bind Texture
	D3D11_TEXTURE2D_DESC desc = { 0 };
	desc.Width = 256;
	desc.Height = 256;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	//desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS; //D3D11_BIND_SHADER_RESOURCE
	desc.CPUAccessFlags = 0; //D3D11_CPU_ACCESS_READ;
	desc.MiscFlags = 0;

	hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &csTexture);
	assert(SUCCEEDED(hr));
	
	//CreateSRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R32G32B32A32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
	
		ID3D11ShaderResourceView* inputSRV;
		hr = D3D::GetDevice()->CreateShaderResourceView(csTexture, &srvDesc, &inputSRV);
	
		material->GetShader()->AsSRV("CSInputTexture")->SetResource(inputSRV);
	}

	//CreateUAV
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		ID3D11UnorderedAccessView* outputUAV;
		hr = D3D::GetDevice()->CreateUnorderedAccessView(csTexture, &uavDesc, &outputUAV);

		material->GetShader()->AsUAV("CSOutputTexture")->SetUnorderedAccessView(outputUAV);
	}
	
	//테스트용 -> 기존 높이팹 텍스쳐 CS용으로 생성
	csHeightTex = new Texture(Contents + L"HeightMaps/HeightMap256.dds");
	material->GetShader()->AsSRV("CSHeightTex")->SetResource(csHeightTex->SRV());
}

//void QuadTree::InitCSDatas(NodeType* node)
//{
//	HRESULT hr;
//	//입력용 버퍼 생성
//	{
//		D3D11_BUFFER_DESC inputDesc;
//		inputDesc.Usage = D3D11_USAGE_DEFAULT;
//		inputDesc.ByteWidth = node->Vertices.size() * sizeof(D3DXVECTOR3);
//		inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//		inputDesc.CPUAccessFlags = 0; //cpu에서 읽지 않으므로 0 
//		inputDesc.StructureByteStride = sizeof(D3DXVECTOR3); //structeredBuffer하나의 크기
//		inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; //버퍼용도
//
//		D3D11_SUBRESOURCE_DATA data1 = { 0 };
//		data1.pSysMem = &node->Vertices[0];
//
//		ID3D11Buffer* buffer1 = NULL;
//		hr = D3D::GetDevice()->CreateBuffer(&inputDesc, &data1, &buffer1);
//		assert(SUCCEEDED(hr)); //TODO : 마우스 누른상태에서 에러
//
//		//뷰 생성
//		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
//		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
//		//structuredBuffer는 BUFFEREX로 설정
//		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
//		srvDesc.BufferEx.FirstElement = 0;
//		srvDesc.BufferEx.Flags = 0;
//		srvDesc.BufferEx.NumElements = node->Vertices.size();
//
//		hr = D3D::GetDevice()->CreateShaderResourceView(buffer1, &srvDesc, &csSrv);
//		assert(SUCCEEDED(hr));
//
//		//뷰 생성후 필요없는 버퍼 삭제
//		SAFE_RELEASE(buffer1);
//	}
//
//	//출력용 순서 없는 접근 뷰와 버퍼 생성
//	{
//		D3D11_BUFFER_DESC outputDesc;
//		outputDesc.Usage = D3D11_USAGE_DEFAULT;
//		outputDesc.ByteWidth = node->Vertices.size() * sizeof(D3DXVECTOR3);
//		outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
//		//view에서 읽을게 아니라 뷰를 받은 버퍼에서 데이터를 읽기 때문에 
//		//뷰에서는 CPU읽기 불가
//		outputDesc.CPUAccessFlags = 0;
//		outputDesc.StructureByteStride = sizeof(D3DXVECTOR3);
//		outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
//		//출력용 버퍼 생성
//		hr = D3D::GetDevice()->CreateBuffer(&outputDesc, NULL, &outputBuffer);
//		assert(SUCCEEDED(hr));
//		//출력용 버퍼의 데이터를 복사할 버퍼 생성
//		outputDesc.Usage = D3D11_USAGE_STAGING;
//		outputDesc.BindFlags = 0;
//		outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//		hr = D3D::GetDevice()->CreateBuffer(&outputDesc, NULL, &outputDebugBuffer);
//		assert(SUCCEEDED(hr));
//		//뷰 생성
//		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
//		uavDesc.Format = DXGI_FORMAT_UNKNOWN; //structuredBuffer를 담기 때문에 UNKNOWN으로 설정?
//		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
//		uavDesc.Buffer.FirstElement = 0;
//		uavDesc.Buffer.Flags = 0;
//		uavDesc.Buffer.NumElements = node->Vertices.size();
//		
//		hr = D3D::GetDevice()->CreateUnorderedAccessView(outputBuffer, &uavDesc, &csUav);
//		assert(SUCCEEDED(hr));
//		//uav에 연결된 outputBuffer의 데이터를 outputDebugBuffer에 복사해서 사용
//		//outputBuffer는 순서없는 접근 뷰에 묶여있어 읽기/쓰기용도로 쓸수 없기때문에
//		//outputBuffer의 데이터를 읽기가능하게 설정한 outputDebugBuffer에 복사하는것이다
//	}
//}
//
//void QuadTree::CopyCSOutput(NodeType * node)
//{
//	node->Vertices.clear(); //size 초기화
//	//node->Vertices.reserve(0); //capacity 초기화
//	//int test = node->Vertices.capacity();
//	//CS의 결과를 OutputDebugBuffer에 복사
//	D3D::GetDC()->CopyResource(outputDebugBuffer, outputBuffer);
//
//	//outputBuffer에서 데이터 읽어와 outputDebugBuffer에 데이터 쓰기
//	D3D11_MAPPED_SUBRESOURCE mapped;
//	D3D::GetDC()->Map(outputDebugBuffer, 0, D3D11_MAP_READ, 0, &mapped);
//	{
//		//명시적 형변환의 경우 캐스팅 될수 없을때 쓰레기값이 들어감
//		//그러므로 리인터프리트캐스트로 캐스팅 될수 없을때 NULL로 채움
//		CSData* dataView = reinterpret_cast<CSData *>(mapped.pData);
//		D3DXVECTOR3 temp = dataView->ResultVertices[1]; //Test용
//		for (int i = 0; i < dataView->ResultVertices.size(); i++) //for (int i = 0; i < dataSize; i++)
//		{
//			//node의 vertices에 데이터 복사
//			//TODO : 여기서 에러
//			//node->Vertices.push_back(dataView->ResultVertices[i]);
//			node->Vertices[i] = dataView->ResultVertices[i];
//		}
//	}
//	D3D::GetDC()->Unmap(outputDebugBuffer, 0);
//
//	//노드의 정점버퍼 재생성
//	node->VertexBuffer = NULL;
//	
//	//버퍼 재생성
//	{
//		D3D11_BUFFER_DESC desc = { 0 };
//		desc.Usage = D3D11_USAGE_DEFAULT; // TODO : 동적버퍼 변경 확인 필요
//		desc.ByteWidth = sizeof(VertexTextureNormalTangent) * node->Vertices.size();
//		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//
//		D3D11_SUBRESOURCE_DATA data = { 0 };
//		data.pSysMem = &node->Vertices[0];
//
//		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &(node->VertexBuffer));
//		assert(SUCCEEDED(hr));
//	}
//}
//
//void QuadTree::DispatchVertices(NodeType * node)
//{
//	vector<UINT> IndexInBrush; //브러쉬 안의 정점의 인덱스
//
//	D3DXVECTOR3 bMin = brush->GetBrushMinPos();
//	D3DXVECTOR3 bMax = brush->GetBrushMaxPos();
//
//	D3DXVECTOR3 currentPos = {0.0f, 0.0f, 0.0f};
//
//	//브러쉬안 정점 판별
//	for (UINT i = 0; i < node->Vertices.size(); i++)
//	{
//		currentPos = node->Vertices[i];
//
//		if (currentPos.x >= bMin.x && currentPos.x <= bMax.x)
//		{
//			if (currentPos.z >= bMin.z && currentPos.z <= bMax.z)
//			{
//				csData->ResultVertices.push_back(currentPos);
//			}
//		}
//	}
//
//	InitCSDatas(node); //node->Vertices.size = 1536
//
//	material->GetShader()->AsSRV("CSInput")->SetResource(csSrv);
//	material->GetShader()->AsUAV("CSOutput")->SetUnorderedAccessView(csUav);
//
//	material->GetShader()->Dispatch(0, 0, 900, 1, 1);
//}
