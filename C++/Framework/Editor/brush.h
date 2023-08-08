#pragma once

class Brush
{
public:
	Brush(class Terrain* terrain);
	~Brush();

	void Initialize(Shader* shader);
	void Ready();
	void Update();
	void Render();

	void InitRect();
	void InitTexture();
	
	bool Picked(OUT D3DXVECTOR3* out, Ray& ray, OUT UINT* pickedIndex);
	
public:
	bool GetbUseBrush() { return pressData->bPress; }
	D3DXVECTOR2 GetBrushMinPos() { return bData->BrushRangeMin; }
	D3DXVECTOR2 GetBrushMaxPos() { return bData->BrushRangeMax; }

private: //범위 지정용 변수들
	VertexTexture* vertices; //브러쉬용 정점
	ID3D11Buffer* vertexBuffer;
	float X, Z;

	D3DXVECTOR3 start, direction;
	D3DXMATRIX world, view, projection;

	class Ray* ray;
	UINT pickedIndex;
	class Terrain* terrain;

private: //텍스쳐 브러쉬 객체들
	Texture* brushTextureArr[20];
	
private:
	struct pixelData
	{
		float Alpha;
		D3DXVECTOR2 PixelPos;
	
		float Padding;
	};
	
	vector<pixelData> pDatas;
	
	struct BrushData
	{
		D3DXVECTOR2 BrushRangeMin;
		D3DXVECTOR2 BrushRangeMax;
	};
	BrushData* bData;
	CBuffer* brushDataBuffer;

	struct TimeData
	{
		float RunningTime;
		float Padding[3];
	};
	TimeData* timeData;
	CBuffer* timeBuffer;

	struct PressData
	{
		bool bPress;

		bool Padding[3];
		float Padding2[3];
	};
	PressData* pressData;
	CBuffer* pressBuffer;

private:
	Shader* shader;

	bool bUseBrush;
	bool bSelectBrush;

	UINT vertexCount;
	float width, height;
	VertexTextureNormalTangent* editVertices; //지형에서 복사해올 정점

	float timeInit;
};