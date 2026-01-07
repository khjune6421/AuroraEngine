#pragma once
#include "Singleton.h"

class TimeManager : public Singleton<TimeManager>
{
	friend class Singleton<TimeManager>;

	DWORD m_previousTime = 0; // 이전 프레임 시간 // 밀리초 단위
	DWORD m_currentTime = 0;  // 현재 프레임 시간 // 밀리초 단위

	float m_timeScale = 1.0f; // 시간 배율
	float m_deltaTime = 0.0f; // 델타 타임 // 초 단위

public:
	~TimeManager() = default;
	TimeManager(const TimeManager&) = delete;
	TimeManager& operator=(const TimeManager&) = delete;
	TimeManager(TimeManager&&) = delete;
	TimeManager& operator=(TimeManager&&) = delete;

	// 타이머 초기화
	void Initialize();
	// 타이머 업데이트
	void UpdateTime();

	void SetTimeScale(float timeScale) { m_timeScale = timeScale; }
	// 델타 타임 얻기
	float GetDeltaTime() const { return m_deltaTime; }

private:
	TimeManager() = default;
};