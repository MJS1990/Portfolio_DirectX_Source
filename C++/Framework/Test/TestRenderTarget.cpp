#include "Framework.h"
#include "TestRenderTarget.h"

TestRenderTarget::TestRenderTarget()
	: vertexCount(4), indexCount(6)
{
}

TestRenderTarget::~TestRenderTarget()
{
}

void TestRenderTarget::Initialize()
{
	material = new Material(Shaders + L"TestRenderTarget.fx");
	quad = new MeshQuad(material);

	//Test Render2D///////////////////////////
	//render1 = new Render2D(Shaders + L"TestRenderTarget.fx");
	render1 = new Render2D();

	D3DDesc desc;
	D3D::GetDesc(&desc);

	render1->Position(0, 0);
	
	//render1->Scale(desc.Width * 0.5f, desc.Height * 0.5f);
	render1->Scale(200, 300);

	//rtvViewport = new Viewport(desc.Width, desc.Height); //rtv¿ë ºäÆ÷Æ®
	/////////////////////////////////////////
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

	//Test RenderTarget
	//D3DDesc desc;
	//D3D::GetDesc(&desc);
	rtv1 = new RenderTargetView(desc.Width, desc.Height);
	//rtv1 = new RenderTargetView(128, 64);
}

void TestRenderTarget::Ready()
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
	material->GetShader()->AsSRV("RenderTargetMap")->SetResource(rtv1->SRV());	
}

void TestRenderTarget::Destroy()
{
	SAFE_DELETE(material);
	SAFE_DELETE(quad);

	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);

	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);

	SAFE_DELETE(texture);

	//TestRenderTarget
	SAFE_DELETE(rtv1);
	//TestRender2D
	SAFE_DELETE(render1);

	SAFE_DELETE(rtvViewport);
}

void TestRenderTarget::Update()
{
}

void TestRenderTarget::PreRender()
{
	D3D::Get()->SetRenderTarget(rtv1->RTV());
	D3D::Get()->Clear(D3DXCOLOR(1, 1, 1, 1), rtv1->RTV());

	//TODO : ºäÆ÷Æ® ¼³Á¤
	//D3DDesc desc;
	//D3D::GetDesc(&desc);
	//rtvViewport->Set(desc.Width * 0.5f, desc.Height * 0.5f);
}

void TestRenderTarget::Render()
{
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;
	
	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	material->SetWorld(world);

	//D3DDesc desc;
	//D3D::GetDesc(&desc);
	//Context::Get()->GetViewport()->Set(desc.Width * 0.5f, desc.Height);
	
	material->GetShader()->DrawIndexed(0, 0, 6); 
}

void TestRenderTarget::PostRender()
{
	//render1->SRV(rtv1->SRV());
	render1->SRV(texture->SRV());
	
	render1->Update();
	
	render1->Render();
}
