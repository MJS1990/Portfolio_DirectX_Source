#pragma once

class Time
{
public:
	static Time* Get();

	static void Create();
	static void Delete();

	static bool Stopped() { return isTimerStopped; }
	static float Delta() { return isTimerStopped ? 0.0f : timeElapsed; }

	void Update();
	void Print();

	void Start();
	void Stop();

	float FPS() const { return framePerSecond; } 
	float Running() const { return runningTime; }

private:
	Time();
	~Time();
	
	static Time* instance;
	//TODO : �ּ� ���� �ʿ�
	static bool isTimerStopped; //Ÿ�̸� ����
	static float timeElapsed; //���� ���������κ����� ����ð�
	
	__time64_t ticksPerSecond; //INT64, �ʴ� ƽ ī��Ʈ
	__time64_t currentTime; //����ð�
	__time64_t lastTime; //�����ð�
	__time64_t lastFPSUpdate; //������ fps������Ʈ �ð�
	__time64_t fpsUpdateInterval; //fps������Ʈ ����
	
	UINT frameCount; //�����Ӽ�
	float runningTime; //����ð�
	float framePerSecond; //fps
};