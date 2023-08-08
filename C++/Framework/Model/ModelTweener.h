#pragma once

//�ִϸ��̼� ����
class ModelTweener
{
public:
	ModelTweener();
	~ModelTweener();

	void Play(class ModelClip* clip, bool bRepeat = false, float blendTime = 0.0f, float speed = 1.0f, float startTime = 0.0f);
	
	//Update�� �� �� �Լ�
	//�ִϸ��̼� ���� ����� ��
	void UpdateBlending(class ModelBone* bone, float time);

private:
	class ModelClip* current;
	class ModelClip* next;

	float blendTime;
	float elapsedTime; //���� �����ӿ��� ���� �����ӱ����� �ð�(=deltaTime) 
};