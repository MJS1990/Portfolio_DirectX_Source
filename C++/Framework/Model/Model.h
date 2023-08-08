#pragma once


//TODO : 오브젝트관리 -> 레퍼런스관리 찾아볼것
class ModelBone;
class ModelMesh;
class Model
{
public:
	Model();
	~Model();
	
	UINT MaterialCount() { return materials.size(); }
	vector<Material *>& Materials() { return materials; }
	Material* MaterialByIndex(UINT index) { return materials[index]; }
	Material* MaterialByName(wstring name);

	UINT BoneCount() { return bones.size(); }
	vector<ModelBone *>& Bones() { return bones; }
	ModelBone* BoneByIndex(UINT index) { return bones[index]; }
	ModelBone* BoneByName(wstring name);

	UINT MeshCount() { return meshes.size(); }
	vector<ModelMesh *>& Meshes() { return meshes; }
	ModelMesh* MeshByIndex(UINT index) { return meshes[index]; }
	ModelMesh* MeshByName(wstring name);

	void ReadMaterial(wstring folder, wstring file);
	void ReadMesh(wstring folder, wstring file);

	//모델본에 각각 존재하는 행렬을 연결하는 함수
	void CopyGlobalBoneTo(vector<D3DXMATRIX>& transforms); //정지모델용
	void CopyGlobalBoneTo(vector<D3DXMATRIX>& transforms, D3DXMATRIX& w); //이동모델용

private:
	void BindingBone(); //부모자식관계 연결
	void BindingMesh(); //부모자식관계 연결

private:
	class ModelBone* root; //rootBone이 될 인스턴스

	vector<Material *> materials;
	vector<class ModelMesh *> meshes;
	vector<class ModelBone *> bones;

};

class Models
{
public:
	friend class Model;

public:
	static void Create();
	static void Delete();

private:
	static void LoadMaterial(wstring file, vector<Material *>* materials);
	static void ReadMaterialData(wstring file);

	static void LoadMesh(wstring file, vector<class ModelBone *>* bones, vector<class ModelMesh *>* meshes);
	static void ReadMeshData(wstring file);

private:
	static map<wstring, vector<Material *>> materialMap;

	struct MeshData
	{
		vector<class ModelBone *> Bones;
		vector<class ModelMesh *> Meshes;
	};

	static map<wstring, MeshData> meshDataMap;
};