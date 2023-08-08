#pragma once

//애니메이션 블렌딩
class ModelTweener
{
public:
	ModelTweener();
	~ModelTweener();

	void Play(class ModelClip* clip, bool bRepeat = false, float blendTime = 0.0f, float speed = 1.0f, float startTime = 0.0f);
	
	//Update시 콜 될 함수
	//애니메이션 섞는 기능을 함
	void UpdateBlending(class ModelBone* bone, float time);

private:
	class ModelClip* current;
	class ModelClip* next;

	float blendTime;
	float elapsedTime; //이전 프레임에서 현재 프레임까지의 시간(=deltaTime) 
};