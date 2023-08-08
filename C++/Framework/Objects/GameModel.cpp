#include "Framework.h"
#include "GameModel.h"

GameModel::GameModel(Shader* shader, wstring matFolder, wstring matFile, wstring meshFolder, wstring meshFile)
{
	model = new Model();
	model->ReadMaterial(matFolder, matFile);
	model->ReadMesh(meshFolder, meshFile);

	model->CopyGlobalBoneTo(transforms, World());
	
	SetShader(shader);
	D3DXMatrixIdentity(&matRotation);

	//�ִϸ��̼� �׽�Ʈ
	//clip = new ModelClip(Models + L"_Kachujin/Samba_Dance.anim");
	clip = new ModelClip(Models + L"_Kachujin/Running.anim");
	clip->LockRoot(false);
	clip->Repeat(true);
	clip->Speed(20.0f);
	clip->StartTime(0.0f);

	anim = new ModelTweener();
}

GameModel::~GameModel()
{
	SAFE_DELETE(model);

	SAFE_DELETE(clip);
	SAFE_DELETE(anim);
}

void GameModel::Update() //������Ʈ �����ʿ�
{
	UpdateAnim();
}

void GameModel::Render()
{
	for (Material* material : model->Materials())
	{
		const float* data = transforms[0];
		UINT count = transforms.size();

		material->GetShader()->AsMatrix("Bones")->SetMatrixArray(data, 0, count);
	}

	for (ModelMesh* mesh : model->Meshes())
		mesh->Render(); //�������� �����ڵ�
}

void GameModel::Rotate(D3DXVECTOR2 amount)
{
	amount *= Time::Delta();

	D3DXMATRIX axis;
	D3DXMatrixRotationAxis(&axis, &Right(), amount.y);

	D3DXMATRIX Y;
	D3DXMatrixRotationY(&Y, Math::ToRadian(amount.x));

	matRotation = axis * Y;

	//D3DXMATRIX R = World();
	//World(matRotation * R);
}

void GameModel::Translate(D3DXVECTOR3 amount)
{
	amount *= Time::Delta();

	D3DXMatrixTranslation(&matLocation, amount.x, amount.y, amount.z);
	World() = matLocation * World();
}

void GameModel::SetShader(Shader * shader)
{
	for (Material* material : model->Materials())
		material->SetShader(shader->GetFile());
}

//srt��ȯ�ɶ��� ��
void GameModel::UpdateWorld()
{
	__super::UpdateWorld();

	D3DXMATRIX W = World();
	model->CopyGlobalBoneTo(transforms, W);
}

void GameModel::UpdateAnim()
{
	//Ŭ���� �̿��� �ϳ��� �ִϸ��̼� ���
	for (int i = 0; i < model->BoneCount(); i++)
	{
		clip->UpdateKeyframe((model->Bones()[i]), Time::Delta());
	}

	//Ʈ������ �̿��� �� �ִϸ��̼� ��ȯ&���
	//for (int i = 0; i < model->BoneCount(); i++)
	//{
	//	//clip->UpdateKeyframe((model->Bones()[i]), Time::Delta());
	//	anim->UpdateBlending((model->Bones()[i]), Time::Delta());
	//}
	//anim->Play(clip, true);	
}

