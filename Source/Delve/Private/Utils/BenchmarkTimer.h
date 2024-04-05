// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <chrono>
#include "CoreMinimal.h"

/**
 * 
 */
class BenchmarkTimer
{
public:
	BenchmarkTimer()
	{
		StartTime = std::chrono::high_resolution_clock::now();
	};
	~BenchmarkTimer()
	{

	};
	int GetTime()
	{
		auto EndTime = std::chrono::high_resolution_clock::now();
		auto Duration = std::chrono::duration_cast<std::chrono::microseconds>(EndTime - StartTime);
		double DurationMilliseconds = Duration.count() / 1000.0;
	};
	void LogTime()
	{
		auto EndTime = std::chrono::high_resolution_clock::now();
		auto Duration = std::chrono::duration_cast<std::chrono::microseconds>(EndTime - StartTime);
		double DurationMilliseconds = Duration.count() / 1000.0;
		//double DurationSeconds = Duration.count() / 1000000.0;
		// Log the duration
		UE_LOG(LogTemp, Warning, TEXT("Execution time: %f milliseconds"), DurationMilliseconds);
	}
private:
	std::chrono::high_resolution_clock::time_point StartTime;
};
