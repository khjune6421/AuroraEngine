#include "stdafx.h"
#include "TimeManager.h"

void TimeManager::Initialize()
{
	m_previousTime = timeGetTime();
}

void TimeManager::UpdateTime()
{
	m_currentTime = timeGetTime();

	constexpr float millisecondsToSeconds = 1.0f / 1000.0f;
	m_deltaTime = static_cast<float>(m_currentTime - m_previousTime) * millisecondsToSeconds * m_timeScale;

	m_previousTime = m_currentTime;
}