#pragma once

class ModelKeyframe
{
public:
	struct Transform;
	friend class ModelClip;

	ModelKeyframe();
	~ModelKeyframe();

public:
	//외부에서 계산한 변환행렬 제공해줄 함수
	D3DXMATRIX GetInterpolatedMatrix(float time, bool bRepeat);

private:
	UINT GetKeyframeIndex(float time);
	void CalcKeyframeIndex(float time, bool bRepeat, OUT UINT& index1, OUT UINT& index2, OUT float& interpolatedTime);
	
	//최종적으로 본에 들어갈 행렬 반환
	D3DXMATRIX GetInterpolatedMatrix(UINT index1, UINT index2, float t);

private:
	wstring boneName;

	float duration; //애니메이션 길이
	float frameRate;
	UINT frameCount;

	vector<Transform> transforms;

public:
	struct Transform
	{
		float Time;

		D3DXVECTOR3 S;
		D3DXQUATERNION R;
		D3DXVECTOR3 T;
	};
private:
	ModelClip* clip;
};