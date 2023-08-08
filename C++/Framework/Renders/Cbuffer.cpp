#include "Framework.h"
#include "CBuffer.h"

CBuffer::CBuffer(Shader * shader, string bufferName, void * pData, UINT dataSize)
	: shader(shader), name(bufferName), data(pData), dataSize(dataSize)
{
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC; //CPU 쓰기, GPU 읽기
	desc.ByteWidth = dataSize;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &buffer);
	assert(SUCCEEDED(hr));

	Change();

	cbVariable = shader->AsConstantBuffer(name);

	CBuffers::buffers.push_back(this);
}

CBuffer::~CBuffer()
{
	SAFE_RELEASE(buffer);
}

void CBuffer::Change()
{
	bChanged = true;
}
//TODO : 단일버퍼 갱신 함수 -> 확인후 놔두거나 삭제or수정
void CBuffer::Change(void * data, UINT dataSize)
{
	D3D11_MAPPED_SUBRESOURCE subResource;
	HRESULT hr = D3D::GetDC()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	assert(SUCCEEDED(hr));

	memcpy(subResource.pData, data, dataSize);
	D3D::GetDC()->Unmap(buffer, 0);

	hr = cbVariable->SetConstantBuffer(buffer);
	assert(SUCCEEDED(hr));
}

void CBuffer::Changed()
{
	if (bChanged == true)
	{
		D3D11_MAPPED_SUBRESOURCE subResource;
		HRESULT hr = D3D::GetDC()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		assert(SUCCEEDED(hr));
	
		memcpy(subResource.pData, data, dataSize);
		D3D::GetDC()->Unmap(buffer, 0);
	
		hr = cbVariable->SetConstantBuffer(buffer);
		assert(SUCCEEDED(hr));
		
		bChanged = false;
	}
}

//CBuffers/////////////////////////////////////////////////////////////////////
vector<CBuffer *> CBuffers::buffers;

void CBuffers::Update()
{
	for (CBuffer* buffer : buffers)
		buffer->Changed();
}
