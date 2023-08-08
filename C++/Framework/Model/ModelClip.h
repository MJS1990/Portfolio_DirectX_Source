#pragma once
#include "ModelKeyframe.h"

class ModelClip
{
public:
	ModelClip(wstring file);
	~ModelClip();

	//bone : 플레이될 본
	D3DXMATRIX GetKeyframeMatrix(class ModelBone* bone, float time);
	void UpdateKeyframe(class ModelBone* bone, float time);

	void LockRoot(bool val) { bLockRoot = val; }
	void Repeat(bool val) { bRepeat = val; }
	void Speed(float val) { speed = val; }
	void StartTime(float val) { playTime = val; }

	void Reset();

private:
	wstring name; //클립 이름
	
	float duration; //애니메이션 길이
	float frameRate; //초당 프레임
	UINT frameCount; //총 프레임

	bool bLockRoot;
	bool bRepeat; //해당 클립이 반복될것인지 아닌지
	float speed; //해당 클립의 재생속도
	float playTime; //애니메이션의 시간경과(밀리세컨드값)

	//vector<class ModelKeyframe *> keyframes;

	typedef pair<wstring, ModelKeyframe *> Pair;
	//unordered_map<본 이름, 이름에 해당하는 키프레임데이터>
	unordered_map<wstring, ModelKeyframe *> keyframeMap;
};