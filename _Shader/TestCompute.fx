struct Data
{
	float3 v1;
	float2 v2;
};

StructuredBuffer<Data> Input1;
StructuredBuffer<Data> Input2;
RWStructuredBuffer<Data> Output;

[numthreads(64, 1, 1)]
void CS(int3 id : SV_GroupThreadId)
{
	Output[id.x].v1 = Input1[id.x].v1 + Input2[id.x].v1;
	Output[id.x].v2 = Input1[id.x].v2 + Input2[id.x].v2;
}

//-----------------------------------------------------------------------------
// Technique
//-----------------------------------------------------------------------------
technique11 T0
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, CS()));
	}
}

/*
ConsumeBuffer, AppendBuffer
입력 등이 순차적이지 않은 데이터
파티클같이 독립적인 연산을 할때 사용
각자의 연산이 자기 데이터만 쓸때
그룹아이디를 사용하지 않고 
예를들어 input1.Consume() 함수를 사용하고
데이터에 맞는 자료형으로 리턴받고(예를 들어 float4로 받으면 float4 a로 리턴받고)
input1은 메모리에서 제거됨
연산이 끝난 후 output.Append()함수로 연산결과를 넣어준다
*/