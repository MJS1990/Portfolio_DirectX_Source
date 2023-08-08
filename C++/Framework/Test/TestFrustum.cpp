#include "Framework.h"
#include "TestFrustum.h"
#include "Viewer/Fixity.h"
#include "Viewer/Frustum.h"

TestFrustum::TestFrustum()
	: drawCount(0)
{
}

TestFrustum::~TestFrustum()
{
}

void TestFrustum::Initialize()
{
	material = new Material();
	material->SetShader(Shaders + L"052_Mesh.fx");
	material->SetDiffuse(D3DXCOLOR(0, 0, 1, 1));

	sphere = new MeshSphere(material, 10);
	sphere->Scale(0.1, 0.1, 0.1);

	camera = new Fixity();
	camera->Position(0, 0, -50);

	projection = new Perspective(1024, 768, Math::PI * 0.25f, 1, 1000);

	//frustum = new Frustum(100, camera, projection);
	frustum = new Frustum(500, Context::Get()->GetMainCamera(), Context::Get()->GetPerspective());

	for (float z = -50; z < 50; z += 10)
	{
		for (float y = -50; y < 50; y += 10)
		{
			for (float x = -50; x < 50; x += 10)
			{
				positions.push_back(D3DXVECTOR3(x, y, z));
			}
		}
	}
}

void TestFrustum::Ready()
{
}

void TestFrustum::Destroy()
{
	SAFE_DELETE(material);
	SAFE_DELETE(sphere);

	SAFE_DELETE(camera);
	SAFE_DELETE(projection);
	SAFE_DELETE(frustum);
}

void TestFrustum::Update()
{
	frustum->Update();
}


void TestFrustum::Render()
{
	drawCount = 0;
	for (D3DXVECTOR3 position : positions)
	{
		if (frustum->ContainPoint(position) == true)
		{
			sphere->Position(position);
			sphere->Render();

			drawCount++;
		}
	}

	ImGui::Text("DrawCount : %d / %d", drawCount, positions.size());
}
