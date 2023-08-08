#include "Framework.h"
#include "DebugLine.h"
#include "ILine.h"

const UINT DebugLine::MaxCount = 5000;

DebugLine::DebugLine()
	: color(0, 0, 0, 1)
{
	shader = new Shader(Shaders + L"Line.fx");

	//Create Vertex Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(Vertex) * MaxCount * 2;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}

	colorVariable = shader->AsVector("Color");
	worldVariable = shader->AsMatrix("World");
}

DebugLine::~DebugLine()
{
	SAFE_DELETE(shader);
	SAFE_RELEASE(vertexBuffer);
}

void DebugLine::Draw(D3DXMATRIX & world, ILine * line)
{
	vector<D3DXVECTOR3> temp;
	line->GetLine(world, temp);

	lines.clear();
	for (size_t i = 0; i < temp.size(); i += 2)
	{
		Line line;
		line.start = temp[i + 0];
		line.end = temp[i + 1];
		lines.push_back(line);
	}

	D3D11_MAPPED_SUBRESOURCE subResource;
	HRESULT hr = D3D::GetDC()->Map
	(
		vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource
	);

	memcpy(subResource.pData, &lines[0], sizeof(Line) * lines.size());

	D3D::GetDC()->Unmap(vertexBuffer, 0);
}

void DebugLine::Color(float r, float g, float b)
{
	Color(D3DXCOLOR(r, g, b, 1.0f));
}

void DebugLine::Color(D3DXCOLOR & vec)
{
	color = vec;
}

void DebugLine::Render()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	colorVariable->SetFloatVector(color);

	D3DXMATRIX w;
	D3DXMatrixIdentity(&w);
	worldVariable->SetMatrix(w);

	shader->Draw(0, 0, lines.size() * 2);
}
