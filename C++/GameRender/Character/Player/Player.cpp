#include "stdafx.h"
#include "Player.h"


Player::Player(float moveSpeed)
	: position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f)
	, bFixCamera(true)
{
	this->moveSpeed = moveSpeed;

	shader = new Shader(Shaders + L"ModelTest.fx");
	//테스트용 모델
	playerModel = new GameModel
	(
		shader,
		Models + L"_Kachujin/", L"Kachujin.material",
		Models + L"_Kachujin/", L"Kachujin.mesh"
	);
	playerModel->Scale(0.05f, 0.05f, 0.05f);
}

Player::~Player()
{
	SAFE_DELETE(shader);
	SAFE_DELETE(playerModel);
}

void Player::Initialize()
{
	//Context::Get()->GetMainCamera()->Position(position.x, position.y + 10, position.z + 17);
	////Context::Get()->GetMainCamera()->RotationDegree(rotation.x, rotation.y);
	////Context::Get()->GetMainCamera()->Position(0, 10, 17);
	//Context::Get()->GetMainCamera()->RotationDegree(10.0f, 180.0f);
}

void Player::Ready()
{
}

void Player::Destroy()
{
}

void Player::Update()
{
	InputMoving();

	if (bFixCamera)
	{
		Context::Get()->GetMainCamera()->Position(position.x, position.y + 10, position.z + 17);
	}

	playerModel->Update();
}

void Player::InputMoving()
{
	if (Keyboard::Get()->Press('W')) //up
	{
		position += playerModel->Direction() * -moveSpeed * Time::Delta();
	}
	if (Keyboard::Get()->Press('S')) //down
	{
		position += playerModel->Direction() * moveSpeed * Time::Delta();
	}
	if (Keyboard::Get()->Press('A')) //left
	{
		position += playerModel->Right() * moveSpeed * Time::Delta();
	}
	if (Keyboard::Get()->Press('D')) //right
	{
		position += playerModel->Right() * -moveSpeed * Time::Delta();
	}

	playerModel->Position(position);
}

void Player::InputRotate()
{
	//TODO : 회전 추가

	playerModel->Rotation(rotation);
}

void Player::PreRender()
{
}

void Player::Render()
{
	playerModel->Render();
}

void Player::PostRender()
{
}

void Player::ResizeScreen()
{
}