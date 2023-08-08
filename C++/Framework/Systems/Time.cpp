#include "Framework.h"
#include "Time.h"

Time* Time::instance = NULL;

bool Time::isTimerStopped = true;
float Time::timeElapsed = 0.0f;

Time * Time::Get()
{
	assert(instance != NULL);

	return instance;
}

void Time::Create()
{
	assert(instance == NULL);

	instance = new Time();
}

void Time::Delete()
{
	SAFE_DELETE(instance);
}

Time::Time()
	: ticksPerSecond(0), currentTime(0), lastTime(0), lastFPSUpdate(0), fpsUpdateInterval(0)
	, frameCount(0), runningTime(0), framePerSecond(0)
{
	//QueryPerformanceFrequency()->�ý����� �����ϴ� ī��Ʈ �ð�(ms)
	//ticksPerSecond�� �ʴ� ī��Ʈ ����
	QueryPerformanceFrequency((LARGE_INTEGER *)&ticksPerSecond);
	fpsUpdateInterval = ticksPerSecond >> 1; // n >> 1 -> n/2
}

Time::~Time()
{
}

void Time::Update()
{
	if (isTimerStopped) return;
	
	//TODO : �ּ� ���� �ʿ�
	//����ð��� ������ �ð� ����, ����ð��� ���
	//QueryPerformanceCounter - ��Ƽ ���μ������� ���� ����
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	timeElapsed = (float)(currentTime - lastTime) / (float)ticksPerSecond;
	runningTime += timeElapsed;

	//fps ������Ʈ
	frameCount++;
	if (currentTime - lastFPSUpdate >= fpsUpdateInterval)
	{
		float tempCurrentTime = (float)currentTime / (float)ticksPerSecond;
		float tempLastTime = (float)lastFPSUpdate / (float)ticksPerSecond;
		framePerSecond = (float)frameCount / (tempCurrentTime - tempLastTime);

		lastFPSUpdate = currentTime;
		frameCount = 0;
	}

	lastTime = currentTime;
}

void Time::Print()
{
}

void Time::Start()
{
	if (!isTimerStopped)
		assert(false);

	QueryPerformanceCounter((LARGE_INTEGER *)&lastTime);
	isTimerStopped = false;
}

void Time::Stop()
{
	if (isTimerStopped)
		assert(false);

	__time64_t stopTime = 0;
	QueryPerformanceCounter((LARGE_INTEGER *)&stopTime);
	runningTime += (float)(stopTime - lastTime) / (float)ticksPerSecond;
	isTimerStopped = true;
}

