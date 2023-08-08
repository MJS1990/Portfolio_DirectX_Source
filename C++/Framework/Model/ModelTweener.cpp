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
	this->blendTime = blendTime; //���� �ð�
	this->elapsedTime = startTime; //���� ������ ���ʿ��� ���۵���

	if (next == NULL) //���� ������ ���ٸ�
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
//���� LocalTransform Tweening �Ǻ�, ��� �� �ٽ� ����
void ModelTweener::UpdateBlending(ModelBone * bone, float time)
{
	D3DXMATRIX matrix;
	if (next != NULL) //���� ������ �����ϸ�
	{
		elapsedTime += time;
		//�ð� ���� ���
		float t = elapsedTime / blendTime;

		if (t > 1.0f) //���� ��ȯ�� �Ϸ�ƴٸ�
		{
			//���� �ִϸ��̼� ���ۿ� ���� ���
			matrix = next->GetKeyframeMatrix(bone, time);
			//������ ��ȯ�����Ƿ� ����ִϸ��̼ǰ� current�� ��ü�� next�� ����
			current = next;
			next = NULL;
		}
		else //��ȯ ���̶�� �� ������ ��ȯ�� �ð�(t)�� ���� �����ؼ� �ε巯�� ���ۺ�ȯ�� ����
		{
			//���� �÷��̵Ǵ� �ִϸ��̼��� ��ȯ���
			D3DXMATRIX start = current->GetKeyframeMatrix(bone, time);
			//���� ���� �ִϸ��̼��� ��ȯ���
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
