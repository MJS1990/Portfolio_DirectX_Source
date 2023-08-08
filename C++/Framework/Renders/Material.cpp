#include "Framework.h"
#include "Material.h"

Material::Material()
	: shader(NULL)
	, diffuseMap(NULL), specularMap(NULL), normalMap(NULL)
	, buffer(NULL)
{
	color.Ambient = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	color.Diffuse = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	color.Specular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	color.Shininess = 0.0f;
}
//�ش� shader�� material�� ����
Material::Material(wstring shaderFile) 
	: shader(NULL), diffuseMap(NULL), specularMap(NULL), normalMap(NULL)
{
	assert(shaderFile.length() > 0);

	color.Ambient = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	color.Diffuse = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	color.Specular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	color.Shininess = 0.0f;

	SetShader(shaderFile);
}

Material::~Material()
{
	SAFE_DELETE(shader);

	SAFE_DELETE(buffer);
	SAFE_DELETE(diffuseMap);
	SAFE_DELETE(specularMap);
	SAFE_DELETE(normalMap);
}

void Material::SetShader(string file)
{
	SetShader(String::ToWString(file));
}

void Material::SetShader(wstring file)
{
	SAFE_DELETE(shader);

	if (file.length() > 0)
	{
		shader = new Shader(file);
		buffer = new CBuffer(shader, "CB_Material", &color, sizeof(ColorDesc));
		
		worldVariable = shader->AsMatrix("World");
		diffuseMapVariable = shader->AsSRV("DiffuseMap");
		specularMapVariable = shader->AsSRV("SpecularMap");
		normalMapVariable = shader->AsSRV("NormalMap");
		
		if (diffuseMap != NULL)
			diffuseMapVariable->SetResource(diffuseMap->SRV());
		
		if (specularMap != NULL)
			specularMapVariable->SetResource(specularMap->SRV());
		
		if (normalMap != NULL)
			normalMapVariable->SetResource(normalMap->SRV());
	}
}

void Material::SetAmbient(D3DXCOLOR & color)
{
	if (this->color.Ambient == color || buffer == NULL)
		return;

	this->color.Ambient = color;
	buffer->Change();
}

void Material::SetAmbient(float r, float g, float b, float a)
{
	SetAmbient(D3DXCOLOR(r, g, b, a));
}

void Material::SetDiffuse(D3DXCOLOR & color)
{
	if (this->color.Diffuse == color || buffer == NULL)
		return;

	this->color.Diffuse = color;
	buffer->Change();
}

void Material::SetDiffuse(float r, float g, float b, float a)
{
	SetDiffuse(D3DXCOLOR(r, g, b, a));
}

void Material::SetSpecular(D3DXCOLOR & color)
{
	if (this->color.Specular == color || buffer == NULL)
		return;

	this->color.Specular = color;
	buffer->Change();
}

void Material::SetSpecular(float r, float g, float b, float a)
{
	SetSpecular(D3DXCOLOR(r, g, b, a));
}

void Material::SetShininess(float val)
{
	if (this->color.Shininess == val || buffer == NULL)
		return;

	this->color.Shininess = val;
	buffer->Change();
}

void Material::SetDiffuseMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetDiffuseMap(String::ToWString(file), info);
}

void Material::SetDiffuseMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	if (diffuseMap != NULL)
	{
		if (diffuseMap->GetFile() == file)
			return;
	}

	SAFE_DELETE(diffuseMap);
	diffuseMap = new Texture(file, info);

	if (shader != NULL)
		diffuseMapVariable->SetResource(diffuseMap->SRV());
}

void Material::SetSpecularMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetSpecularMap(String::ToWString(file), info);
}

void Material::SetSpecularMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	if (specularMap != NULL)
	{
		if (specularMap->GetFile() == file)
			return;
	}

	SAFE_DELETE(specularMap)
		specularMap = new Texture(file, info);

	if (shader != NULL)
		specularMapVariable->SetResource(specularMap->SRV());
}

void Material::SetNormalMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetNormalMap(String::ToWString(file), info);
}

void Material::SetNormalMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	if (normalMap != NULL)
	{
		if (normalMap->GetFile() == file)
			return;
	}

	SAFE_DELETE(normalMap);
	normalMap = new Texture(file, info);

	if (shader != NULL)
		normalMapVariable->SetResource(normalMap->SRV());
}

void Material::SetWorld(D3DXMATRIX & val)
{
	worldVariable->SetMatrix(val);
}

void Material::Copy(Material ** clone)
{
	Material* material = new Material();
	material->name = this->name;

	if (this->shader != NULL)
		material->SetShader(this->shader->GetFile());

	material->SetDiffuse(this->GetDiffuse());
	material->SetSpecular(this->GetSpecular());

	if (this->diffuseMap != NULL)
		material->SetDiffuseMap(this->diffuseMap->GetFile());

	if (this->specularMap != NULL)
		material->SetSpecularMap(this->specularMap->GetFile());

	if (this->normalMap != NULL)
		material->SetNormalMap(this->normalMap->GetFile());

	material->SetShininess(this->GetShininess());

	*clone = material;
}
