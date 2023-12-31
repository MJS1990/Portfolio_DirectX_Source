#include "Framework.h"
#include "ModelClip.h"
#include "ModelBone.h"
#include "../Utilities/BinaryFile.h"

ModelClip::ModelClip(wstring file)
	: bRepeat(false), speed(1.0f), playTime(0.0f), bLockRoot(false)
{
	BinaryReader* r = new BinaryReader();
	r->Open(file);
	
	name = String::ToWString(r->String());
	duration = r->Float();
	frameRate = r->Float();
	frameCount = r->UInt();

	UINT keyframesCount = r->UInt();
	for (UINT i = 0; i < keyframesCount; i++)
	{
		ModelKeyframe* keyframe = new ModelKeyframe();
		keyframe->boneName = String::ToWString(r->String());

		keyframe->duration = duration;
		keyframe->frameCount = frameCount;
		keyframe->frameRate = frameRate;

		UINT size = r->UInt();
		if (size > 0)
		{
			keyframe->transforms.assign(size, ModelKeyframe::Transform());

			void* ptr = (void *)&keyframe->transforms[0];
			r->Byte(&ptr, sizeof(ModelKeyframe::Transform) * size);
		}

		keyframeMap.insert(Pair(keyframe->boneName, keyframe));
	}

	r->Close();
	SAFE_DELETE(r);
}

ModelClip::~ModelClip()
{
	for (Pair keyframe : keyframeMap)
		SAFE_DELETE(keyframe.second);
}

void ModelClip::Reset()
{
	bRepeat = false;
	speed = 1.0f;
	playTime = 0.0f;
}

//
D3DXMATRIX ModelClip::GetKeyframeMatrix(ModelBone * bone, float time)
{
	wstring boneName = bone->Name();
	unordered_map<wstring, ModelKeyframe *>::iterator it;
	//undorderedMap.find에서 값을 찾지못하면 end가 들어오기 때문에 map안에 boneName과 
	//일치하는 본이 있는지 검사
	//TODO : 여기서 에러, keyframeMap데이터 확인해볼것
	if ((it = keyframeMap.find(boneName)) == keyframeMap.end()) //값을 찾지 못했다면 단위행렬 반환
	{
		D3DXMATRIX temp;
		D3DXMatrixIdentity(&temp);

		return temp;
	}

	ModelKeyframe* keyframe = keyframeMap.at(boneName);

	playTime += speed * time;
	if (bRepeat == true)
	{
		if (playTime >= duration)
		{
			while (playTime - duration >= 0)
				playTime -= duration;
		}
	}
	else
	{
		if (playTime >= duration)
			playTime = duration;
	}

	D3DXMATRIX invGlobal = bone->Global();
	D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

	D3DXMATRIX animation = keyframe->GetInterpolatedMatrix(playTime, bRepeat);

	D3DXMATRIX parent;
	int parentIndex = bone->ParentIndex();
	if (parentIndex < 0)
	{
		if (bLockRoot == true)
			D3DXMatrixIdentity(&parent);
		else
			parent = animation;
	}
	else
	{
		parent = animation * bone->Parent()->Global();
	}
	
	return invGlobal * parent;
}

void ModelClip::UpdateKeyframe(ModelBone* bone, float time)
{
	D3DXMATRIX animation = GetKeyframeMatrix(bone, time);
	
	bone->Local(animation);
}