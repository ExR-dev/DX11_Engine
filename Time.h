#pragma once

#include <chrono>


class Time
{
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	std::chrono::time_point<std::chrono::high_resolution_clock> frame;

public:
	float time, deltaTime;

	Time();

	void Update();
};