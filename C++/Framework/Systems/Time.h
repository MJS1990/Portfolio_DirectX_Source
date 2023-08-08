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
	//TODO : 주석 삭제 필요
	static bool isTimerStopped; //타이머 중지
	static float timeElapsed; //이전 프레임으로부터의 경과시간
	
	__time64_t ticksPerSecond; //INT64, 초당 틱 카운트
	__time64_t currentTime; //현재시간
	__time64_t lastTime; //이전시간
	__time64_t lastFPSUpdate; //마지막 fps업데이트 시간
	__time64_t fpsUpdateInterval; //fps업데이트 간격
	
	UINT frameCount; //프레임수
	float runningTime; //진행시간
	float framePerSecond; //fps
};