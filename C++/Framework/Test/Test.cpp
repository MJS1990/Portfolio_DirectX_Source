#include "Framework.h"
#include "Test.h"

Test* Test::instance = NULL;

Test * Test::Get()
{
	if (instance == NULL)
		Create();

	assert(instance != NULL);

	return instance;
}

void Test::Create()
{
	assert(instance == NULL);

	instance = new Test();
}

void Test::Delete()
{
	SAFE_DELETE(instance);
}

void Test::Update()
{
}

Test::Test()
{
}

Test::~Test()
{
}
