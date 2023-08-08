#pragma once
#include "Mesh.h"

class MeshHemiSphere : public Mesh
{
public:
	MeshHemiSphere(Material* material, UINT drawCount = 32);
	~MeshHemiSphere();

	void CreateData() override;

private:
	UINT drawCount;
};
