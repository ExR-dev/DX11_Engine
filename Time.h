#pragma once

#include <chrono>
#include <intsafe.h>

struct Snapshot
{
	std::string name;
	std::chrono::time_point<std::chrono::high_resolution_clock> snapshot;
};

class Time
{
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _start;
	std::chrono::time_point<std::chrono::high_resolution_clock> _frame;

	std::vector<Snapshot> _snapshots;

public:
	float time, deltaTime;

	Time();

	void Update();

	UINT TakeSnapshot(const std::string &name);
	[[nodiscard]] float CompareSnapshots(UINT s1, UINT s2) const;
	[[nodiscard]] float CompareSnapshots(const std::string &name) const;
};
