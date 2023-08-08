#pragma once
#include "Systems/IExecute.h"

class TestModel : public IExecute
{
public:
	TestModel();
	~TestModel();

	void Initialize();
	void Ready() {}
	void Destroy() {}

	void Update();
	void PreRender();
	void Render();
	void PostRender();

	void ResizeScreen() {}

private:
	Shader* shader;
	//GameModel* toilet;
	GameModel* testModel;
};