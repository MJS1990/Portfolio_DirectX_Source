#include "Framework.h"
#include "TestRenderTarget2.h"

//TestRenderTarget 쉐이더에서 0번 백버퍼 렌더링과 같이 rtv의 RenderTargetView에도 texture 렌더링
//rtv1의 SRV를 Render2D객체에 셋
//PostRender에서 Render2D 렌더

TestRenderTarget2::TestRenderTarget2()
	: vertexCount(4), indexCount(6)
{
}

TestRenderTarget2::~TestRenderTarget2()
{
}

void TestRenderTarget2::Initialize()
{
	material = new Material(Shaders + L"TestRenderTarget.fx");
	
	D3DXMatrixIdentity(&world);

	//Texture
	texture = new Texture(Textures + L"tree.dds");

	vertices = new VertexTexture[vertexCount];
	vertices[0].Position = D3DXVECTOR3(-0.5f, -0.5f, 0.0f);
	vertices[1].Position = D3DXVECTOR3(-0.5f,  0.5f, 0.0f);
	vertices[2].Position = D3DXVECTOR3( 0.5f, -0.5f, 0.0f);
	vertices[3].Position = D3DXVECTOR3( 0.5f,  0.5f, 0.0f);

	vertices[0].Uv = D3DXVECTOR2(0.0f, 1.0f);
	vertices[1].Uv = D3DXVECTOR2(0.0f, 0.0f);
	vertices[2].Uv = D3DXVECTOR2(1.0f, 1.0f);
	vertices[3].Uv = D3DXVECTOR2(1.0f, 0.0f);
	
	indices = new UINT[indexCount]{ 0, 1, 2, 2, 1, 3 };


	//RenderTarget
	{
		D3DDesc desc;
		D3D::GetDesc(&desc);
		//rtv = new RenderTargetView(200, 300);
		rtv = new RenderTargetView(desc.Width, desc.Height);
		render2d = new Render2D();
		
		render2d->Position(0.0f, 0.0f);
		render2d->Scale(320, 150);
		//render2d->Position(desc.Width * 0.5f, 0);
		//render2d->Scale(desc.Width * 0.5f, desc.Height);

		rtvViewport = new Viewport(desc.Width, desc.Height);
	}
}

void TestRenderTarget2::Ready()
{
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

	material->GetShader()->AsSRV("TestMap")->SetResource(texture->SRV());
	material->GetShader()->AsSRV("RenderTargetMap")->SetResource(rtv->SRV());

	material->GetShader()->AsSampler("TargetSampler")->SetSampler(11, targetSampler);
}

void TestRenderTarget2::Destroy()
{
	SAFE_DELETE(material);

	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);

	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);

	SAFE_DELETE(texture);

	SAFE_DELETE(rtv);
	SAFE_DELETE(render2d);

	SAFE_DELETE(rtvViewport);

	targetSampler->Release();
	SAFE_RELEASE(targetSampler);
}

void TestRenderTarget2::Update()
{
}

void TestRenderTarget2::PreRender()
{
	rtv->Set(D3DXCOLOR(1, 1, 1, 1));

	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;
	
	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	material->SetWorld(world);

	material->GetShader()->DrawIndexed(0, 1, 6);
	//material->GetShader()->DrawIndexed(0, 1, 6);

	//D3D::GetDC()->DrawIndexed(6, 0, 0); //TODO : 추가 렌더링 확인
}

void TestRenderTarget2::Render()
{
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	material->SetWorld(world);

	material->GetShader()->DrawIndexed(0, 0, 6);
	//material->GetShader()->DrawIndexed(0, 1, 6);

	//D3D::GetDC()->DrawIndexed(6, 0, 1); //TODO : 추가 렌더링 확인
}

void TestRenderTarget2::PostRender()
{
	render2d->SRV(rtv->SRV());
	//render2d->SRV(texture->SRV());

	//render2d->Update();

	render2d->Render();
}