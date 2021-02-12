#pragma once
#include <string>
#include <chrono>

#define TIMEFUNC() CBTimer TimedFunc(__FUNCTION__)

class CBTimer
{
public:
	CBTimer(const std::string& InName);
    ~CBTimer();
	void Stop();

private:
    CBTimer() = delete;

	std::chrono::steady_clock::time_point StartTime;
	std::string Name;
	bool bStopped = false;
};