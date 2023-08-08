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
	//QueryPerformanceFrequency()->시스템이 지원하는 카운트 시간(ms)
	//ticksPerSecond에 초당 카운트 대입
	QueryPerformanceFrequency((LARGE_INTEGER *)&ticksPerSecond);
	fpsUpdateInterval = ticksPerSecond >> 1; // n >> 1 -> n/2
}

Time::~Time()
{
}

void Time::Update()
{
	if (isTimerStopped) return;
	
	//TODO : 주석 삭제 필요
	//현재시간을 가져와 시간 간격, 진행시간을 계산
	//QueryPerformanceCounter - 멀티 프로세서에서 사용시 주의
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	timeElapsed = (float)(currentTime - lastTime) / (float)ticksPerSecond;
	runningTime += timeElapsed;

	//fps 업데이트
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

