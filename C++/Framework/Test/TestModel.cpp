#include "Framework.h"
#include "TestModel.h"

TestModel::TestModel()
{
	shader = new Shader(Shaders + L"ModelTest.fx");

	////TODO : 파일확인하고 Export먼저 하고 실행
	//toilet = new GameModel
	//(
	//	shader, 
	//	Models + L"Toilet/", L"Toilet.material",
	//	Models + L"Toilet/", L"Toilet.mesh"
	//);
	//toilet->Scale(0.05f, 0.05f, 0.05f);
	//toilet->Position(0.0f, 0.0f, 1.0f);
	////toilet->SetDiffuseMap(Textures + L"White.png"); //TODO : 수업땐 있던 코드

	//테스트용 모델
	testModel = new GameModel
	(
		shader,
		Models + L"_Kachujin/", L"Kachujin.material",
		Models + L"_Kachujin/", L"Kachujin.mesh"
	);
	testModel->Scale(0.05f, 0.05f, 0.05f);
	//testModel->SetDiffuseMap(Textures + L"White.png"); //TODO : 수업땐 있던 코드
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
	////TODO : Export먼저 하고 실행
	//toilet = new GameModel
	//(
	//	shader,
	//	Materials + L"Meshes/", L"Toilet.material",
	//	Models + L"Meshes/", L"Toilet.mesh"
	//);
	//toilet->Scale(1.0f, 1.0f, 1.0f);
	////toilet->SetDiffuseMap(Textures + L"White.png"); //TODO : 수업땐 있던 코드
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
