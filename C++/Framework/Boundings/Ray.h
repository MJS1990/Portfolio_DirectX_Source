#pragma once
#include "Framework.h"
#include "../Draw/ILine.h"

class Ray : public ILine
{
public:
	Ray()
	{
		Position = Direction = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}

	Ray(D3DXVECTOR3& position, D3DXVECTOR3& direction)
		: Position(position), Direction(direction)
	{

	}

	void GetLine(D3DXMATRIX& world, vector<D3DXVECTOR3>& lines)
	{
		lines.push_back(Position);
		lines.push_back(Position + (Direction * Distance));
	}

	D3DXVECTOR3 Position;
	D3DXVECTOR3 Direction;
	float Distance = 10.0f;
};