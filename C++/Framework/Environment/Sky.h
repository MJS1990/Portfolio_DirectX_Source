#pragma once

class Sky
{
public:
	Sky();
	~Sky();

	void Initialize();
	void Ready();
	void Destroy();

	void Update();
	void PreRender();
	void Render();
	void PostRender();

private:
	void GenerateSphere();
	void GenerateQuad();

private:
	MeshHemiSphere* sphere;
	MeshQuad* quad;

private:
	bool realTime; 
	float theta, phi;
	float prevTheta, prevPhi;

	RenderTargetView* mieTarget, *rayleighTarget;
	Shader* scatterBuffer;
	
	Texture* starField;
	ID3D11SamplerState* rayleighSampler;
	ID3D11SamplerState* mieSampler;

	Material* material;

	//HemiSphere, Quad
	UINT vertexCount, indexCount;
	UINT radius;
	UINT slices, stacks;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	VertexTexture* quadVertices;
	ID3D11Buffer* quadBuffer;
	//

	Shader* shader;

	Render2D* rayleigh2D;
	Render2D* mie2D;

private:
	struct TargetBuffer
	{
		D3DXVECTOR3 WaveLength;
		int SampleCount;

		D3DXVECTOR3 InvWaveLength;
		float Padding1;

		D3DXVECTOR3 WaveLengthMie;
		float Padding2;
	};

	TargetBuffer tBuffer;

	CBuffer* targetBuffer;
};