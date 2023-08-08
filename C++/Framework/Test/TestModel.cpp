#include "Framework.h"
#include "TestModel.h"

TestModel::TestModel()
{
	shader = new Shader(Shaders + L"ModelTest.fx");

	////TODO : ����Ȯ���ϰ� Export���� �ϰ� ����
	//toilet = new GameModel
	//(
	//	shader, 
	//	Models + L"Toilet/", L"Toilet.material",
	//	Models + L"Toilet/", L"Toilet.mesh"
	//);
	//toilet->Scale(0.05f, 0.05f, 0.05f);
	//toilet->Position(0.0f, 0.0f, 1.0f);
	////toilet->SetDiffuseMap(Textures + L"White.png"); //TODO : ������ �ִ� �ڵ�

	//�׽�Ʈ�� ��
	testModel = new GameModel
	(
		shader,
		Models + L"_Kachujin/", L"Kachujin.material",
		Models + L"_Kachujin/", L"Kachujin.mesh"
	);
	testModel->Scale(0.05f, 0.05f, 0.05f);
	//testModel->SetDiffuseMap(Textures + L"White.png"); //TODO : ������ �ִ� �ڵ�
}

TestModel::~TestModel()
{
	SAFE_DELETE(shader);

	//SAFE_DELETE(toilet);
	SAFE_DELETE(testModel);
}

void TestModel::Initialize()
{
	//shader = new Shader(Shaders + L"TestModel.fx");
	//
	////TODO : Export���� �ϰ� ����
	//toilet = new GameModel
	//(
	//	shader,
	//	Materials + L"Meshes/", L"Toilet.material",
	//	Models + L"Meshes/", L"Toilet.mesh"
	//);
	//toilet->Scale(1.0f, 1.0f, 1.0f);
	////toilet->SetDiffuseMap(Textures + L"White.png"); //TODO : ������ �ִ� �ڵ�
}

void TestModel::Update()
{
	//toilet->Update();
	testModel->Update();
}

void TestModel::PreRender()
{
}

void TestModel::Render()
{
	//toilet->Render();
	testModel->Render();
}

void TestModel::PostRender()
{
}
