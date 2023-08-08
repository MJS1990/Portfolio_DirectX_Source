#include "Framework.h"
#include "Sky.h"

Sky::Sky()
	: realTime(false), theta(0), phi(0)
	, radius(10), slices(32), stacks(16)
{
}

Sky::~Sky()
{
}

void Sky::Initialize()
{
	shader = new Shader(Shaders + L"Scattering3.fx");	
	
	rayleighTarget = new RenderTargetView(128, 64);
	mieTarget = new RenderTargetView(128, 64);

	//Render2D Setting
	rayleigh2D = new Render2D();
	rayleigh2D->Position(0, 100);
	rayleigh2D->Scale(200, 100);

	mie2D = new Render2D();
	mie2D->Position(0, 0);
	mie2D->Scale(200, 100);

	GenerateSphere();
	GenerateQuad();

	//targetBuffer
	{
		tBuffer.WaveLength = D3DXVECTOR3(0.65f, 0.57f, 0.475f);
		tBuffer.SampleCount = 20; //샘플링 20번

		tBuffer.InvWaveLength.x = 1.0f / powf(tBuffer.WaveLength.x, 4.0f);
		tBuffer.InvWaveLength.y = 1.0f / powf(tBuffer.WaveLength.y, 4.0f);
		tBuffer.InvWaveLength.z = 1.0f / powf(tBuffer.WaveLength.z, 4.0f);

		tBuffer.WaveLengthMie.x = powf(tBuffer.WaveLength.x, -0.84f);
		tBuffer.WaveLengthMie.y = powf(tBuffer.WaveLength.y, -0.84f);
		tBuffer.WaveLengthMie.z = powf(tBuffer.WaveLength.z, -0.84f);
	}
	targetBuffer = new CBuffer(shader, "TargetBuffer", &tBuffer, sizeof(TargetBuffer));
}

void Sky::Ready()
{
}

void Sky::Destroy()
{
	SAFE_DELETE(targetBuffer);

	SAFE_DELETE(rayleighTarget);
	SAFE_DELETE(mieTarget);
	//HemiSphere, Quad
	SAFE_DELETE_ARRAY(quadVertices);
	SAFE_RELEASE(quadBuffer);

	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);
	//

	SAFE_DELETE(rayleigh2D);
	SAFE_DELETE(mie2D);

	SAFE_DELETE(shader);
}

void Sky::Update()
{
	D3DXMATRIX V;
	D3DXVECTOR3 position;
	Context::Get()->GetMainCamera()->Position(&position);
	D3DXMatrixTranslation(&V, position.x, position.y, position.z);
	
	shader->AsMatrix("World")->SetMatrix(V);
}

void Sky::PreRender()
{
	if (prevTheta == theta && prevPhi == phi)
		return;

	//RenderTarget Setting
	mieTarget->Set();
	rayleighTarget->Set();

	ID3D11RenderTargetView* rtvs[2];
	rtvs[0] = rayleighTarget->RTV();
	rtvs[1] = mieTarget->RTV();

	D3D::Get()->SetRenderTargets(2, rtvs);
	
	//RenderTarget Draw
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &quadBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->Draw(0, 1, 6);
}

void Sky::Render()
{
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;
	
	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->DrawIndexed(0, 0, indexCount);

	shader->AsSRV("RayleighMap")->SetResource(rayleighTarget->SRV());
	shader->AsSRV("MieMap")->SetResource(mieTarget->SRV());
}

void Sky::PostRender()
{
	//Test RenderTarget
	rayleigh2D->SRV(rayleighTarget->SRV());
	rayleigh2D->Render();
	
	mie2D->SRV(mieTarget->SRV());
	mie2D->Render();
}

void Sky::GenerateSphere()
{
	UINT drawCount = 32;
	UINT latitude = drawCount / 2; //위도 (남<->북)
	UINT longitude = drawCount; //경도 (서<->동)

	vertexCount = longitude * latitude * 2;
	indexCount = (longitude - 1) * (latitude - 1) * 2 * 8;

	VertexTexture* vertices = new VertexTexture[vertexCount];

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
	UINT* indices = new UINT[indexCount];
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
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = offset + i * latitude + j;
			indices[index++] = offset + (i + 1) * latitude + j + 1;
			indices[index++] = offset + (i + 1) * latitude + j;

			indices[index++] = offset + i * latitude + j + 1;
			indices[index++] = offset + (i + 1) * latitude + j + 1;
			indices[index++] = offset + i * latitude + j;
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

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}

	//CreateIndexBuffer
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

	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);
}

void Sky::GenerateQuad()
{
	quadVertices = new VertexTexture[6];

	quadVertices[0].Position = D3DXVECTOR3(-1.0f, -1.0f,  0.0f);
	quadVertices[1].Position = D3DXVECTOR3(-1.0f,  1.0f,  0.0f);
	quadVertices[2].Position = D3DXVECTOR3( 1.0f, -1.0f,  0.0f);
	quadVertices[3].Position = D3DXVECTOR3( 1.0f, -1.0f,  0.0f);
	quadVertices[4].Position = D3DXVECTOR3(-1.0f,  1.0f,  0.0f);
	quadVertices[5].Position = D3DXVECTOR3( 1.0f,  1.0f,  0.0f);

	quadVertices[0].Uv = D3DXVECTOR2(0, 1);
	quadVertices[1].Uv = D3DXVECTOR2(0, 0);
	quadVertices[2].Uv = D3DXVECTOR2(1, 1);
	quadVertices[3].Uv = D3DXVECTOR2(1, 1);
	quadVertices[4].Uv = D3DXVECTOR2(0, 0);
	quadVertices[5].Uv = D3DXVECTOR2(1, 0);


	//CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTexture) * 6;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = quadVertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &quadBuffer);
		assert(SUCCEEDED(hr));
	}
}
