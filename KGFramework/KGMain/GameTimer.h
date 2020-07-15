#pragma once
#pragma once
#include <Windows.h>
namespace KG
{
	constexpr ULONG MAX_SAMPLE_COUNT = 50;
	class GameTimer
	{
	public:
		GameTimer();
		virtual ~GameTimer();
		void Tick(float fLockFPS = 0.0f);
		unsigned long GetFrameRate();
		float GetTimeElapsed();
		void Reset();
	private:
		double timeScale;
		float timeElapsed;

		__int64 basePerformanceCounter;
		__int64 pausedPerformanceCounter;
		__int64 stopPerformanceCounter;

		union
		{
			__int64 currentPerformanceCounter;
			__int64 currentTime;
		};
		union
		{
			__int64 lastPerformanceCounter;
			__int64 lastTime;
		};

		__int64 performanceFrequencyPerSec;



		float frameTime[MAX_SAMPLE_COUNT];
		ULONG sampleCount;
		unsigned long currentFrameRate;
		unsigned long framePerSecond;
		float fpsTimeElapsed;
	};
};