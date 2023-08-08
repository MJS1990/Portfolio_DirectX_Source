#include "Framework.h"
#include "Model.h"
#include "ModelMesh.h"
#include "ModelMeshPart.h"
#include "ModelBone.h"
#include "../Utilities/Xml.h"
#include "../Utilities/BinaryFile.h"


//ReadMaterial과 ReadMesh를 같은 이름의 파일이 
//이미 로드되어 있으면 그 파일을 가져와서 사용하게 변경
void Model::ReadMaterial(wstring folder, wstring file)
{
	Models::LoadMaterial(folder + file, &materials);
}

void Model::ReadMesh(wstring folder, wstring file)
{
	Models::LoadMesh(folder + file, &bones, &meshes);

	//본과 매쉬의 부모자식관계 연결
	BindingBone();
	BindingMesh(); //TODO : 여기서 에러
}

void Model::BindingBone()
{
	this->root = bones[0];
	for (ModelBone* bone : bones)
	{
		if (bone->parentIndex > -1) //자식 본이면
		{
			bone->parent = bones[bone->parentIndex];
			bone->parent->childs.push_back(bone);
		}
		else //rootBone이면
		{
			bone->parent = NULL;
		}
	}
}

void Model::BindingMesh()
{
	for (ModelMesh* mesh : meshes)
	{
		for (ModelBone* bone : bones)
		{
			if (mesh->parentBoneIndex == bone->index)
			{
				mesh->parentBone = bone;
				break;
			}
		}

		for (ModelMeshPart* part : mesh->meshParts) 
		{
			for (Material* material : materials) 
			{
				wstring test = material->Name();
				if (material->Name() == part->materialName)
				{
					part->material = material;
					break;
				}
			}
		}

		mesh->Binding(); //meshPart의 Vertex,Index 정보 생성
	}
}

/////////////////////////////////////////////////////////////////////////////

map<wstring, vector<Material *>> Models::materialMap;

void Models::LoadMaterial(wstring file, vector<Material*>* materials)
{
	if (materialMap.count(file) < 1) //해당 file에 material이 없으면 파일에서 읽어들임
		ReadMaterialData(file);

	for (Material* material : materialMap[file]) //동일한 material이 있으면 데이터 복사
	{
		Material* temp = NULL;
		//material->Clone((void**)&temp); //TODO : 이 부분 실행되게 수정
		material->Copy(&temp); //TODO : Clone이 Copy로 대체됨

		materials->push_back(temp);
	}
}

void Models::ReadMaterialData(wstring file)
{
	vector<Material *> materials;

	Xml::XMLDocument* document = new Xml::XMLDocument();

	wstring tempFile = file;
	Xml::XMLError error = document->LoadFile(String::ToString(tempFile).c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement* root = document->FirstChildElement();
	Xml::XMLElement* matNode = root->FirstChildElement(); //루트아래의 첫번째 메테리얼

	do
	{
		Xml::XMLElement* node = NULL;

		Material* material = new Material();
		//Read FileName
		node = matNode->FirstChildElement();
		material->Name(String::ToWString(node->GetText()));

		node = node->NextSiblingElement();

		//Read diffuse
		D3DXCOLOR dxColor;
		Xml::XMLElement* color = node->FirstChildElement();
		dxColor.r = color->FloatText();

		color = color->NextSiblingElement();
		dxColor.g = color->FloatText();

		color = color->NextSiblingElement();
		dxColor.b = color->FloatText();

		color = color->NextSiblingElement();
		dxColor.a = color->FloatText();
		material->SetDiffuse(dxColor);

		//Read DiffuseFile
		node = node->NextSiblingElement();
		wstring diffuseTexture = String::ToWString(node->GetText());

		wstring directory = Path::GetDirectoryName(file); //폴더명만 가져옴

		if (diffuseTexture.length() > 0)
			material->SetDiffuseMap(directory + diffuseTexture);
	
		materials.push_back(material);
		matNode = matNode->NextSiblingElement();
	} while (matNode != NULL);

	materialMap[file] = materials;
}

///////////////////////////////////////////////////////////////////////////////

map<wstring, Models::MeshData> Models::meshDataMap;

void Models::LoadMesh(wstring file, vector<class ModelBone*>* bones, vector<class ModelMesh*>* meshes)
{
	if (meshDataMap.count(file) < 1)
		ReadMeshData(file);

	MeshData data = meshDataMap[file];
	for (size_t i = 0; i < data.Bones.size(); i++)
	{
		ModelBone* bone = NULL;
		data.Bones[i]->Clone((void**)&bone);

		bones->push_back(bone);
	}

	for (size_t i = 0; i < data.Meshes.size(); i++)
	{
		ModelMesh* mesh = NULL;
		data.Meshes[i]->Clone((void**)&mesh);

		meshes->push_back(mesh);
	}
}

void Models::ReadMeshData(wstring file)
{
	BinaryReader* r = new BinaryReader();
	r->Open(file);

	vector<ModelBone *> bones;
	vector<ModelMesh *> meshes;

	UINT count = 0;
	count = r->UInt();

	for (UINT i = 0; i < count; i++)
	{
		//Read Bone
		ModelBone* bone = new ModelBone();

		bone->index = r->Int();
		bone->name = String::ToWString(r->String());
		bone->parentIndex = r->Int();

		bone->local = r->Matrix();
		bone->global = r->Matrix();

		bones.push_back(bone);
	}

	//Read Mesh
	count = r->UInt();
	for (UINT i = 0; i < count; i++)
	{
		ModelMesh* mesh = new ModelMesh();
		mesh->name = String::ToWString(r->String());

		mesh->parentBoneIndex = r->Int();

		UINT partCount = r->UInt();
		for (UINT k = 0; k < partCount; k++)
		{
			//Read MeshPart
			ModelMeshPart* meshPart = new ModelMeshPart();
			meshPart->parent = mesh;
			meshPart->materialName = String::ToWString(r->String());

			//Read Vertex
			{
				UINT count = r->UInt();
				meshPart->vertices.assign(count, ModelVertexType());

				void* ptr = (void *)&(meshPart->vertices[0]);
				r->Byte(&ptr, sizeof(ModelVertexType) * count);
			}

			//Read Index
			{
				UINT count = r->UInt();
				meshPart->indices.assign(count, UINT());

				void* ptr = (void *)&(meshPart->indices[0]);
				r->Byte(&ptr, sizeof(UINT) * count);
			}

			mesh->meshParts.push_back(meshPart);
		}
		meshes.push_back(mesh);
	}

	r->Close();
	SAFE_DELETE(r);


	MeshData data;
	data.Bones.assign(bones.begin(), bones.end());
	data.Meshes.assign(meshes.begin(), meshes.end());

	meshDataMap[file] = data;
}
