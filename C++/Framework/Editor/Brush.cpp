#include "Framework.h"
#include "Brush.h"
#include "Environment/Terrain.h"

Brush::Brush(Terrain* terrain)
	: terrain(terrain)
	, X(15.0f), Z(15.0f), timeInit(0.0f)
	, bUseBrush(false), bSelectBrush(false)
{	
}

void Brush::Initialize(Shader* shader)
{
	this->shader = shader;

	width = terrain->GetWidth();
	height = terrain->GetHeight();
	world = terrain->GetWorld();

	vertexCount = terrain->VertexCount();
	editVertices = new VertexTextureNormalTangent[vertexCount];
	terrain->CopyVertices((void *)editVertices);

	InitRect();
	InitTexture();

	ray = new Ray();

	//BrushData초기화, Buffer
	bData = new BrushData();
	bData->BrushRangeMin.x = vertices[0].Position.x;
	bData->BrushRangeMin.y = vertices[0].Position.z;
	bData->BrushRangeMax.x = vertices[5].Position.x;
	bData->BrushRangeMax.y = vertices[5].Position.z;
	brushDataBuffer = new CBuffer(shader, "BrushDataBuffer", bData, sizeof(BrushData));

	//TimeData초기화, Buffer
	timeData = new TimeData();
	timeData->RunningTime = 0.0f;
	timeBuffer = new CBuffer(shader, "TimeBuffer", timeData, sizeof(TimeData));

	//PressData초기화, Buffer
	pressData = new PressData();
	pressData->bPress = false;
	pressBuffer = new CBuffer(shader, "PressBuffer", pressData, sizeof(PressData));

	//ReadBrushPixel(19);
	shader->AsSRV("BrushTexture")->SetResource(brushTextureArr[13]->SRV());
}

void Brush::Ready()
{
}

Brush::~Brush()
{
	SAFE_DELETE(bData);
	SAFE_DELETE(brushDataBuffer);
	
	SAFE_DELETE(timeData);
	SAFE_DELETE(timeBuffer);

	SAFE_DELETE(pressData);
	SAFE_DELETE(pressBuffer);

	SAFE_DELETE(ray);
	SAFE_DELETE_ARRAY(*brushTextureArr);
}


void Brush::Update()
{
	if (Mouse::Get()->Press(0))
	{
		pressData->bPress = true;
		pressBuffer->Change();
		timeData->RunningTime = Time::Get()->Running() - timeInit;
		timeBuffer->Change();
	}
	else //if (Mouse::Get()->Up(0))
	{
		pressData->bPress = false;
		pressBuffer->Change();
	
		timeInit = Time::Get()->Running();
	}

	////브러쉬 크기 변경
	//ImGui::InputFloat("BrushX", &X, 1.0f, 100.0f);
	//ImGui::InputFloat("BrushZ", &Z, 1.0f, 100.0f);

	D3DXVECTOR3 center; //브러쉬 이동값
	Picked(&center, *ray, &pickedIndex);

	D3DXMATRIX T;
	D3DXMatrixTranslation(&T, center.x, 0.0f, center.z);
	
	shader->AsMatrix("BrushWorld")->SetMatrix(T);

	//BrushRange Update
	bData->BrushRangeMin.x = center.x + vertices[0].Position.x;
	bData->BrushRangeMin.y = center.z + vertices[0].Position.z;
	bData->BrushRangeMax.x = center.x + vertices[5].Position.x;
	bData->BrushRangeMax.y = center.z + vertices[5].Position.z;
	brushDataBuffer->Change();

	//지형 범위 넘어가면 범위 재지정
	if (center.x > terrain->GetWidth())
		center.x = terrain->GetWidth();
	else if (center.x < 0.0f)
		center.x = 0.0f;

	if (center.z > terrain->GetHeight())
		center.z = terrain->GetHeight();
	else if (center.z < 0.0f)
		center.x = 0.0f;
	
	//브러쉬 선택 UI//
	ImGui::Checkbox("Use Brush", &bUseBrush);
	if(bUseBrush == true)
	{
		for (UINT i = 0; i < 20; i++)
		{
			//버튼표시 & 선택된 버튼 식별
			bSelectBrush = ImGui::ImageButton(brushTextureArr[i]->SRV(), ImVec2(50.0f, 50.0f));
			if (i != 0 && i % 5 == 0) //버튼 수평 표시
				ImGui::SameLine();
			//TODO : 선택된 SRV를 쉐이더에 바인딩
			if (bSelectBrush == true) //브러쉬 텍스쳐중 하나를 선택했다면
			{
				shader->AsSRV("BrushTexture")->SetResource(brushTextureArr[i]->SRV());
			}//if(SelectBrush)
			bSelectBrush = false;
		}//for(i)
	}//if(bUseBrush)

	ImGui::InputFloat2("BrushV0", bData->BrushRangeMin);
	ImGui::InputFloat2("BrushV5", bData->BrushRangeMax);
	ImGui::InputInt("Mouse", (int*)&pressData->bPress);
}

void Brush::Render()
{
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->DrawIndexed(0, 1, 6);
}

void Brush::InitRect()
{
	vertices = new VertexTexture[6];
	vertices[0].Position = D3DXVECTOR3(-X,  1.0f, -Z);
	vertices[1].Position = D3DXVECTOR3(-X,  1.0f,  Z);
	vertices[2].Position = D3DXVECTOR3( X,  1.0f, -Z);
	vertices[3].Position = D3DXVECTOR3( X,  1.0f, -Z);
	vertices[4].Position = D3DXVECTOR3(-X,  1.0f,  Z);
	vertices[5].Position = D3DXVECTOR3( X,  1.0f,  Z);

	vertices[0].Uv = D3DXVECTOR2(0.0f, 1.0f);
	vertices[1].Uv = D3DXVECTOR2(0.0f, 0.0f);
	vertices[2].Uv = D3DXVECTOR2(1.0f, 1.0f);
	vertices[3].Uv = D3DXVECTOR2(1.0f, 1.0f);
	vertices[4].Uv = D3DXVECTOR2(0.0f, 0.0f);
	vertices[5].Uv = D3DXVECTOR2(1.0f, 0.0f);

	//Create VertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTexture) * 6;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}
}

void Brush::InitTexture()
{
	brushTextureArr[0] =  new Texture(Textures + L"Brush/brush_0.dds");
	brushTextureArr[1] =  new Texture(Textures + L"Brush/brush_1.dds");
	brushTextureArr[2] =  new Texture(Textures + L"Brush/brush_2.dds");
	brushTextureArr[3] =  new Texture(Textures + L"Brush/brush_3.dds");
	brushTextureArr[4] =  new Texture(Textures + L"Brush/brush_4.dds");
	brushTextureArr[5] =  new Texture(Textures + L"Brush/brush_5.dds");
	brushTextureArr[6] =  new Texture(Textures + L"Brush/brush_6.dds");
	brushTextureArr[7] =  new Texture(Textures + L"Brush/brush_7.dds");
	brushTextureArr[8] =  new Texture(Textures + L"Brush/brush_8.dds");
	brushTextureArr[9] =  new Texture(Textures + L"Brush/brush_9.dds");
	brushTextureArr[10] = new Texture(Textures + L"Brush/brush_10.dds");
	brushTextureArr[11] = new Texture(Textures + L"Brush/brush_11.dds");
	brushTextureArr[12] = new Texture(Textures + L"Brush/brush_12.dds");
	brushTextureArr[13] = new Texture(Textures + L"Brush/brush_13.dds");
	brushTextureArr[14] = new Texture(Textures + L"Brush/brush_14.dds");
	brushTextureArr[15] = new Texture(Textures + L"Brush/brush_15.dds");
	brushTextureArr[16] = new Texture(Textures + L"Brush/brush_16.dds");
	brushTextureArr[17] = new Texture(Textures + L"Brush/brush_17.dds");
	brushTextureArr[18] = new Texture(Textures + L"Brush/brush_18.dds");
	brushTextureArr[19] = new Texture(Textures + L"Brush/brush_19.dds");
}

bool Brush::Picked(OUT D3DXVECTOR3 * out, Ray & ray, OUT UINT * pickedIndex)
{
	Context::Get()->GetMainCamera()->Position(&start);

	//TODO : Ray추가
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
			index[0] = (UINT)((((width * 6) * z)) + i);
			index[1] = (UINT)((((width * 6) * z)) + i + 1);
			index[2] = (UINT)((((width * 6) * z)) + i + 2);
			index[3] = (UINT)((((width * 6) * z)) + i + 5);

			D3DXVECTOR3 p[4];
			for (int i = 0; i < 4; i++)
				p[i] = editVertices[index[i]].Position;

			float u, v, distance;
			if (D3DXIntersectTri(&p[0], &p[1], &p[2], &ray.Position, &direction, &u, &v, &ray.Distance))
			{
				*out = p[0] + (p[1] - p[0]) * u + (p[2] - p[0]) * v;
				*pickedIndex = index[0] / 6;
				return true;
			}

			if (D3DXIntersectTri(&p[3], &p[1], &p[2], &ray.Position, &direction, &u, &v, &ray.Distance))
			{
				*out = p[3] + (p[1] - p[3]) * u + (p[2] - p[3]) * v;
				*pickedIndex = (index[3] - 5) / 6;
				return true;
			}
		}
	}

	return false;
}