#include "Framework.h"
#include "Test/TestGeometry.h"
#include "Environment/Terrain.h"

TestGeometry::TestGeometry()
{
	terrain = new Terrain();
}

TestGeometry::~TestGeometry()
{
}

void TestGeometry::Initialize()
{
	terrain->Initialize();
}

void TestGeometry::Ready()
{
	terrain->Ready();
}

void TestGeometry::Destroy()
{
	terrain->Destroy();
	SAFE_DELETE(terrain);
}

void TestGeometry::Update()
{
	terrain->Update();
}

void TestGeometry::PreRender()
{
}

void TestGeometry::Render()
{
	terrain->Render();
}

void TestGeometry::PostRender()
{
}

void TestGeometry::ResizeScreen()
{
}
