#pragma once
#include "GameRender.h"

#include "../Model/Model.h"
#include "../Model/ModelBone.h"
#include "../Model/ModelMesh.h"
//�ִϸ��̼�
#include "../Model/ModelClip.h"
#include "../Model/ModelTweener.h"

class GameModel : public GameRender
{
public:
	GameModel(Shader* shader, wstring matFolder, wstring matFile, wstring meshFolder, wstring meshFile);
	virtual ~GameModel();

	virtual void Update();
	virtual void Render();

	void Translate(D3DXVECTOR3 amount);
	void Rotate(D3DXVECTOR2 amount); //z�� ȸ�� ����

	void SetShader(Shader* shader);
	//void SetAnimation();

private:
	void UpdateWorld() override;
	void UpdateAnim();

private:
	//�ִϸ��̼� �׽�Ʈ��
	ModelClip* clip;
	ModelTweener* anim;
	float x, y, z;

protected:
	Model* model;
	
	D3DXMATRIX matLocation; //�̵� ���
	D3DXMATRIX matRotation; //ȸ�� ��� //TODO : ���󿡼� ����, �����ʿ�
	vector<D3DXMATRIX> transforms;
};