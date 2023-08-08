#pragma once
#include "GameRender.h"

#include "../Model/Model.h"
#include "../Model/ModelBone.h"
#include "../Model/ModelMesh.h"
//애니메이션
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
	void Rotate(D3DXVECTOR2 amount); //z축 회전 제외

	void SetShader(Shader* shader);
	//void SetAnimation();

private:
	void UpdateWorld() override;
	void UpdateAnim();

private:
	//애니메이션 테스트용
	ModelClip* clip;
	ModelTweener* anim;
	float x, y, z;

protected:
	Model* model;
	
	D3DXMATRIX matLocation; //이동 행렬
	D3DXMATRIX matRotation; //회전 행렬 //TODO : 영상에선 없음, 수정필요
	vector<D3DXMATRIX> transforms;
};