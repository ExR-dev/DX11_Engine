#include "Time.h"


Time::Time() : start(std::chrono::high_resolution_clock::now()), frame(std::chrono::high_resolution_clock::now())
{
	time = 0.0f;
	deltaTime = 1.0f / 60.0f;
}

void Time::Update()
{
	const auto newFrame = std::chrono::high_resolution_clock::now();

	const std::chrono::duration<float> newTime = newFrame - start;
	time = newTime.count();

	const std::chrono::duration<float> newDeltaTime = newFrame - frame;
	deltaTime = newDeltaTime.count();

	frame = newFrame;
}