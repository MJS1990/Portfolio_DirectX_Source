#include "Framework.h"
#include "ModelTweener.h"
#include "ModelClip.h"
#include "ModelBone.h"

ModelTweener::ModelTweener()
	: current(NULL), next(NULL), blendTime(0.0f), elapsedTime(0.0f)
{

}

ModelTweener::~ModelTweener()
{
}

void ModelTweener::Play(ModelClip * clip, bool bRepeat, float blendTime, float speed, float startTime)
{
	this->blendTime = blendTime; //섞일 시간
	this->elapsedTime = startTime; //다음 동작이 몇초에서 시작될지

	if (next == NULL) //다음 동작이 없다면
	{
		next = clip;
		next->Repeat(bRepeat);
		next->StartTime(startTime);
		next->Speed(speed);
	}
	else 
	{
		current = clip;
		current->Repeat(bRepeat);
		current->StartTime(startTime);
		current->Speed(speed);
	}
}
//본의 LocalTransform Tweening 판별, 계산 후 다시 저장
void ModelTweener::UpdateBlending(ModelBone * bone, float time)
{
	D3DXMATRIX matrix;
	if (next != NULL) //다음 동작이 존재하면
	{
		elapsedTime += time;
		//시간 비율 계산
		float t = elapsedTime / blendTime;

		if (t > 1.0f) //동작 전환이 완료됐다면
		{
			//다음 애니메이션 동작에 대한 행렬
			matrix = next->GetKeyframeMatrix(bone, time);
			//동작을 전환했으므로 현재애니메이션값 current에 교체한 next를 대입
			current = next;
			next = NULL;
		}
		else //전환 중이라면 두 동작의 변환을 시간(t)에 따라 보간해서 부드러운 동작변환을 수행
		{
			//현재 플레이되는 애니메이션의 변환행렬
			D3DXMATRIX start = current->GetKeyframeMatrix(bone, time);
			//다음 섞을 애니메이션의 변환행렬
			D3DXMATRIX end = next->GetKeyframeMatrix(bone, time);

			Math::LerpMatrix(matrix, start, end, t);
		}
	}
	else
	{
		matrix = current->GetKeyframeMatrix(bone, time);
	}

	bone->Local(matrix);
}
