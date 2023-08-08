#pragma once
#include "ModelKeyframe.h"

class ModelClip
{
public:
	ModelClip(wstring file);
	~ModelClip();

	//bone : �÷��̵� ��
	D3DXMATRIX GetKeyframeMatrix(class ModelBone* bone, float time);
	void UpdateKeyframe(class ModelBone* bone, float time);

	void LockRoot(bool val) { bLockRoot = val; }
	void Repeat(bool val) { bRepeat = val; }
	void Speed(float val) { speed = val; }
	void StartTime(float val) { playTime = val; }

	void Reset();

private:
	wstring name; //Ŭ�� �̸�
	
	float duration; //�ִϸ��̼� ����
	float frameRate; //�ʴ� ������
	UINT frameCount; //�� ������

	bool bLockRoot;
	bool bRepeat; //�ش� Ŭ���� �ݺ��ɰ����� �ƴ���
	float speed; //�ش� Ŭ���� ����ӵ�
	float playTime; //�ִϸ��̼��� �ð����(�и������尪)

	//vector<class ModelKeyframe *> keyframes;

	typedef pair<wstring, ModelKeyframe *> Pair;
	//unordered_map<�� �̸�, �̸��� �ش��ϴ� Ű�����ӵ�����>
	unordered_map<wstring, ModelKeyframe *> keyframeMap;
};