#pragma once

class GameRender
{
public:
	GameRender();
	virtual ~GameRender();

	D3DXMATRIX World();

	void Position(D3DXVECTOR3& vec);
	void Position(float x, float y, float z);
	D3DXVECTOR3 Position();

	void Scale(D3DXVECTOR3& vec);
	void Scale(float x, float y, float z);
	D3DXVECTOR3 Scale();

	void Rotation(D3DXVECTOR3& vec);
	void Rotation(float x, float y, float z);
	void RotationDegree(D3DXVECTOR3& vec);
	void RotationDegree(float x, float y, float z);
	D3DXVECTOR3 Rotation();
	D3DXVECTOR3 RotationDegree();

	D3DXVECTOR3 Direction();
	D3DXVECTOR3 Up();
	D3DXVECTOR3 Right();

	virtual void Update() = 0;
	virtual void Render() = 0;

protected:
	virtual void UpdateWorld();

private:
	D3DXMATRIX world;

	D3DXVECTOR3 position;
	D3DXVECTOR3 scale;
	D3DXVECTOR3 rotation;

	D3DXVECTOR3 direction;
	D3DXVECTOR3 up;
	D3DXVECTOR3 right;
};