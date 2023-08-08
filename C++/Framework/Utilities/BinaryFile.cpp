#include "Framework.h"
#include "BinaryFile.h"

//BinaryWriter/////////////////////////////////////////////////////////////////

BinaryWriter::BinaryWriter()
	: fileHandle(NULL), size(0)
{
}

BinaryWriter::~BinaryWriter()
{
}

void BinaryWriter::Open(wstring filePath, UINT openOption)
{
	assert(filePath.length() > 0);
	fileHandle = CreateFile //파일, 오브젝트를 생성하거나 여는 함수
	(
		filePath.c_str() //파일을 열거나 생성할 경로
		, GENERIC_WRITE //열기 쓰기할때 액세스 권한
		, 0 //파일의 공유 모드
		, NULL //SECURITY_ATTRIBUTES구조체의 포인터
		, openOption //파일의 해당 위치존재 유무에 따른 행동
		, FILE_ATTRIBUTE_NORMAL //생성된 파일의 속성을 지정(읽기전용, 숨김 등등)
		, NULL //파일이 읽기권한일때(GENERIC_READ)의 탬플릿파일의 유효한 핸들, 생성된 파일에 대한 속성을 제공
	);
	//만들어진 핸들이 초기화 값과 같으면 false
	bool isChecked = fileHandle != INVALID_HANDLE_VALUE;
	assert(isChecked);
}

void BinaryWriter::Close()
{
	if (fileHandle != NULL)
	{
		CloseHandle(fileHandle); //핸들 닫는 함수
		fileHandle = NULL;
	}
}

void BinaryWriter::Bool(bool data)
{
	//파일 데이터를 쓰는 함수
	//1. 파일이나 장치의 핸들(반드시 쓰기 권한이 있어야 한다)
	//2. 파일이나 장치에 기록할 데이터를 가지고 있는 버퍼의 포인터
	//3. 기록할 데이터의 크기
	//4. 기록한 데이터의 바이트수를 리턴받을 인수(함수가 호출된 시점에 0으로 초기화)
	//5. 비동기 입출력을 위한 OVERLAPPED구조체의 포인터
	WriteFile(fileHandle, &data, sizeof(bool), &size, NULL);
}

void BinaryWriter::Word(WORD data)
{
	WriteFile(fileHandle, &data, sizeof(WORD), &size, NULL);
}

void BinaryWriter::Int(int data)
{
	WriteFile(fileHandle, &data, sizeof(int), &size, NULL);
}

void BinaryWriter::UInt(UINT data)
{
	WriteFile(fileHandle, &data, sizeof(UINT), &size, NULL);
}

void BinaryWriter::Float(float data)
{
	WriteFile(fileHandle, &data, sizeof(float), &size, NULL);
}

void BinaryWriter::Double(double data)
{
	WriteFile(fileHandle, &data, sizeof(double), &size, NULL);
}

void BinaryWriter::Vector2(const D3DXVECTOR2 & data)
{
	WriteFile(fileHandle, &data, sizeof(D3DXVECTOR2), &size, NULL);
}

void BinaryWriter::Vector3(const D3DXVECTOR3 & data)
{
	WriteFile(fileHandle, &data, sizeof(D3DXVECTOR3), &size, NULL);
}

void BinaryWriter::Vector4(const D3DXVECTOR4 & data)
{
	WriteFile(fileHandle, &data, sizeof(D3DXVECTOR4), &size, NULL);
}

void BinaryWriter::Color3f(const D3DXCOLOR & data)
{
	WriteFile(fileHandle, &data, sizeof(D3DXCOLOR) - 4, &size, NULL);
}

void BinaryWriter::Color4f(const D3DXCOLOR & data)
{
	WriteFile(fileHandle, &data, sizeof(D3DXCOLOR), &size, NULL);
}

void BinaryWriter::Matrix(const D3DXMATRIX & data)
{
	WriteFile(fileHandle, &data, sizeof(D3DXMATRIX), &size, NULL);
}

void BinaryWriter::String(const string & data)
{
	UInt(data.size());

	const char* str = data.c_str();
	WriteFile(fileHandle, str, data.size(), &size, NULL);
}

void BinaryWriter::Byte(void * data, UINT dataSize)
{
	WriteFile(fileHandle, &data, dataSize, &size, NULL);
}

//BinaryReader/////////////////////////////////////////////////////////////////

BinaryReader::BinaryReader()
	: fileHandle(NULL), size(0)
{
}

BinaryReader::~BinaryReader()
{
}

void BinaryReader::Open(wstring filePath)
{
	assert(filePath.length() > 0);
	fileHandle = CreateFile
	(
		filePath.c_str()
		, GENERIC_READ
		, FILE_SHARE_READ
		, NULL
		, OPEN_EXISTING
		, FILE_ATTRIBUTE_NORMAL
		, NULL
	);

	bool isChecked = fileHandle != INVALID_HANDLE_VALUE;
	assert(isChecked);
}

void BinaryReader::Close()
{
	if (fileHandle != NULL)
	{
		CloseHandle(fileHandle);
		fileHandle = NULL;
	}
}

bool BinaryReader::Bool()
{
	bool temp = false;
	ReadFile(fileHandle, &temp, sizeof(bool), &size, NULL);

	return temp;
}

WORD BinaryReader::Word()
{
	WORD temp = 0; 
	//파일데이터를 읽는 함수
	//1. 파일의 핸들
	//2. 파일로부터 읽은 데이터를 받아올 버퍼의 포인터
	//3. 읽어들일 데이터의 크기
	//4. 읽어들인 데이터의 바이트 수를 리턴받을 인수
	//5. 비동기 입출력을 위한 OVERLAPPED 구조체의 포인터
	ReadFile(fileHandle, &temp, sizeof(WORD), &size, NULL);

	return temp;
}

int BinaryReader::Int()
{
	int temp = 0;
	ReadFile(fileHandle, &temp, sizeof(int), &size, NULL);

	return temp;
}

UINT BinaryReader::UInt()
{
	UINT temp = 0;
	ReadFile(fileHandle, &temp, sizeof(UINT), &size, NULL);

	return temp;
}

float BinaryReader::Float()
{
	float temp = 0.0f;
	ReadFile(fileHandle, &temp, sizeof(float), &size, NULL);

	return temp;
}

double BinaryReader::Double()
{
	double temp = 0.0f;
	ReadFile(fileHandle, &temp, sizeof(double), &size, NULL);

	return temp;
}

D3DXVECTOR2 BinaryReader::Vector2()
{
	float x = Float();
	float y = Float();

	return D3DXVECTOR2(x, y);
}

D3DXVECTOR3 BinaryReader::Vector3()
{
	float x = Float();
	float y = Float();
	float z = Float();

	return D3DXVECTOR3(x, y, z);
}

D3DXVECTOR4 BinaryReader::Vector4()
{
	float x = Float();
	float y = Float();
	float z = Float();
	float w = Float();

	return D3DXVECTOR4(x, y, z, w);
}

D3DXCOLOR BinaryReader::Color3f()
{
	float r = Float();
	float g = Float();
	float b = Float();

	return D3DXCOLOR(r, g, b, 1.0f);
}

D3DXCOLOR BinaryReader::Color4f()
{
	float r = Float();
	float g = Float();
	float b = Float();
	float a = Float();

	return D3DXCOLOR(r, g, b, a);
}

D3DXMATRIX BinaryReader::Matrix()
{
	D3DXMATRIX matrix;
	matrix._11 = Float(); matrix._12 = Float(); matrix._13 = Float(); matrix._14 = Float();
	matrix._21 = Float(); matrix._22 = Float(); matrix._23 = Float(); matrix._24 = Float();
	matrix._31 = Float(); matrix._32 = Float(); matrix._33 = Float(); matrix._34 = Float();
	matrix._41 = Float(); matrix._42 = Float(); matrix._43 = Float(); matrix._44 = Float();

	return matrix;
}

string BinaryReader::String()
{
	UINT size = Int();

	char* temp = new char[size + 1];
	ReadFile(fileHandle, temp, sizeof(char) * size, &this->size, NULL);
	temp[size] = '\0';

	return temp;
}

void BinaryReader::Byte(void ** data, UINT dataSize)
{
	ReadFile(fileHandle, *data, dataSize, &size, NULL);
}
