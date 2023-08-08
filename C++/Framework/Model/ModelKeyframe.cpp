#include "Framework.h"
#include "ModelKeyframe.h"

ModelKeyframe::ModelKeyframe()
{
}

ModelKeyframe::~ModelKeyframe()
{
}
//TODO : �ܺο��� �� �� �Լ�
D3DXMATRIX ModelKeyframe::GetInterpolatedMatrix(float time, bool bRepeat)
{
	//index1 : �÷��̵� ������
	//index2 : �÷��̵� ���� ������
	//interpolatedTime : index1���� index2���� �帥 �ð��� ������ ��
	UINT index1 = 0, index2 = 0;
	float interpolatedTime = 0.0f;
	//time-> �ִϸ��̼��� �ð������(�и�������)
	CalcKeyframeIndex(time, bRepeat, index1, index2, interpolatedTime);
	
	return GetInterpolatedMatrix(index1, index2, interpolatedTime);
}

//�ش� �ð��� �÷����� Ű ������ ���ϴ� �Լ�
UINT ModelKeyframe::GetKeyframeIndex(float time) 
{
	UINT start = 0, end = frameCount - 1;

	//time�� �ִϸ��̼��� �� ���̺��� ũ�ų� ������ ������ ������ ��ȯ
	if (time >= transforms[end].Time) 
		return end;

	do //���� �ݾ� ���������鼭 �ش� �������� �ð������� �ε����� ����
	{
		UINT middle = (start + end) / 2;

		if (end - start <= 1)
		{
			break;
		}
		else if(transforms[middle].Time < time)
		{
			start = middle;
		}
		else if (transforms[middle].Time > time)
		{
			end = middle;
		}
		else
		{
			start = middle;

			break;
		}

	} while ((end - start) > 1);

	return start;
}

//index1 : �÷��̵� ������
//index2 : �÷��̵� ���� ������
//interpolatedTime : index1���� index2���� �帥 �ð��� ������ ��
void ModelKeyframe::CalcKeyframeIndex(float time, bool bRepeat, OUT UINT & index1, OUT UINT & index2, OUT float & interpolatedTime)
{
	index1 = index2 = 0;
	interpolatedTime = 0.0f;

	index1 = GetKeyframeIndex(time);
	//index2�� ����
	if (bRepeat == false) //�ѹ� ����϶�
		index2 = (index1 >= frameCount - 1) ? index1 : index1 + 1; //index1�� �������� ���� �����ϸ�
	else //�ݺ�����϶�
		index2 = (index1 >= frameCount - 1) ? 0 : index1 + 1; //index1�� �������� ���� �����ϸ�

	//�ش� �ִϸ��̼� �ð������� 0~1���̷� ������ �ð����� ����
	if (index1 >= frameCount - 1) //�ִϸ��̼��� ������ �����̶��
	{
		index1 = index2 = frameCount - 1;

		interpolatedTime = 1.0f;
	}
	else
	{
		float time1 = time - transforms[index1].Time;
		float time2 = transforms[index2].Time - transforms[index1].Time;

		interpolatedTime = time1 / time2;
	}
}
//���ϰ��� �ϴ� �ð������� �����Ӱ�, �ٷ� ������ �����Ӱ��� 
//�� �ð������� 0~1���� ������ �ð������� �������� SRT ��ȯ����� ���ѵ� ��ȯ
D3DXMATRIX ModelKeyframe::GetInterpolatedMatrix(UINT index1, UINT index2, float t)
{
	D3DXVECTOR3 scale;
	D3DXVec3Lerp(&scale, &transforms[index1].S, &transforms[index2].S, t);
	D3DXMATRIX S;
	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	
	D3DXQUATERNION q;
	D3DXMATRIX R;
	D3DXQuaternionSlerp(&q, &transforms[index1].R, &transforms[index2].R, t);
	D3DXMatrixRotationQuaternion(&R, &q);

	D3DXVECTOR3 translation;
	D3DXMATRIX T;
	D3DXVec3Lerp(&translation, &transforms[index1].T, &transforms[index2].T, t);
	D3DXMatrixTranslation(&T, translation.x, translation.y, translation.z);

	return S * R * T;
}