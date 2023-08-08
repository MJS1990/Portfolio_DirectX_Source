#include "Framework.h"
#include "ModelKeyframe.h"

ModelKeyframe::ModelKeyframe()
{
}

ModelKeyframe::~ModelKeyframe()
{
}
//TODO : 외부에서 콜 될 함수
D3DXMATRIX ModelKeyframe::GetInterpolatedMatrix(float time, bool bRepeat)
{
	//index1 : 플레이될 프레임
	//index2 : 플레이될 다음 프레임
	//interpolatedTime : index1부터 index2까지 흐른 시간을 보간한 값
	UINT index1 = 0, index2 = 0;
	float interpolatedTime = 0.0f;
	//time-> 애니메이션의 시간경과값(밀리세컨드)
	CalcKeyframeIndex(time, bRepeat, index1, index2, interpolatedTime);
	
	return GetInterpolatedMatrix(index1, index2, interpolatedTime);
}

//해당 시간에 플레이할 키 프레임 구하는 함수
UINT ModelKeyframe::GetKeyframeIndex(float time) 
{
	UINT start = 0, end = frameCount - 1;

	//time이 애니메이션의 총 길이보다 크거나 같으면 마지막 프레임 반환
	if (time >= transforms[end].Time) 
		return end;

	do //범위 반씩 좁혀나가면서 해당 프레임의 시간에서의 인덱스를 구함
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

//index1 : 플레이될 프레임
//index2 : 플레이될 다음 프레임
//interpolatedTime : index1부터 index2까지 흐른 시간을 보간한 값
void ModelKeyframe::CalcKeyframeIndex(float time, bool bRepeat, OUT UINT & index1, OUT UINT & index2, OUT float & interpolatedTime)
{
	index1 = index2 = 0;
	interpolatedTime = 0.0f;

	index1 = GetKeyframeIndex(time);
	//index2값 구함
	if (bRepeat == false) //한번 재생일때
		index2 = (index1 >= frameCount - 1) ? index1 : index1 + 1; //index1이 프레임의 끝에 도달하면
	else //반복재생일때
		index2 = (index1 >= frameCount - 1) ? 0 : index1 + 1; //index1이 프레임의 끝에 도달하면

	//해당 애니메이션 시간에서의 0~1사이로 보간된 시간값을 구함
	if (index1 >= frameCount - 1) //애니메이션의 마지막 동작이라면
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
//구하고자 하는 시간에서의 프레임값, 바로 다음의 프레임값과 
//그 시간에서의 0~1까지 보간된 시간값으로 최종적인 SRT 변환행렬을 구한뒤 반환
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