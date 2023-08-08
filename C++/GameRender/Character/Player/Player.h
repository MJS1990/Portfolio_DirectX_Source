#pragma once
#include "Systems/IExecute.h"

class Player : public IExecute
{
public:
	Player(float moveSpeed = 500.0f);
	~Player();

	void Initialize() override;
	void Ready() override;
	void Destroy() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void PostRender() override;
	void ResizeScreen() override;

public:
	void SetMoveSpeed(float  val) { moveSpeed = val; }
	D3DXVECTOR3 GetPosition() { return playerModel->Position(); }
	D3DXVECTOR3 GetRotation() { return playerModel->Rotation(); }

	void FixCamera(bool val) { bFixCamera = val; }

private:
	void InputMoving();
	void InputRotate();

private:
	Shader* shader;
	GameModel* playerModel;

private:
	D3DXVECTOR3 position;
	D3DXVECTOR3 rotation;

	float moveSpeed;
	bool bFixCamera;
};

class PlayerState
{
public:
	PlayerState() { currentState = eState::Idle; }
	~PlayerState() {}

	enum eState
	{
		None = -1,
		Idle = 0,
		Walk,
		Run,
		Attack_0,
		Attack_1,
		Attack_2,
		Attack_3,
		Attack_4,
		Damaged,
		Dead,
	};

public:
	eState GetState() { return currentState; }

private:
	eState currentState;
};