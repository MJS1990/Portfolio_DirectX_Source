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
�Է� ���� ���������� ���� ������
��ƼŬ���� �������� ������ �Ҷ� ���
������ ������ �ڱ� �����͸� ����
�׷���̵� ������� �ʰ� 
������� input1.Consume() �Լ��� ����ϰ�
�����Ϳ� �´� �ڷ������� ���Ϲް�(���� ��� float4�� ������ float4 a�� ���Ϲް�)
input1�� �޸𸮿��� ���ŵ�
������ ���� �� output.Append()�Լ��� �������� �־��ش�
*/