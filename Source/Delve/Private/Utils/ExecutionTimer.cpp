// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils/ExecutionTimer.h"

UExecutionTimer::UExecutionTimer()
    : bIsRunning(false)
{
}

void UExecutionTimer::Start()
{
    StartTime = FDateTime::UtcNow();
    bIsRunning = true;
}

float UExecutionTimer::Stop()
{
    if (bIsRunning)
    {
        EndTime = FDateTime::UtcNow();
        bIsRunning = false;
        return (EndTime - StartTime).GetTotalMilliseconds();
    }
    return 0.0f;
}

float UExecutionTimer::GetElapsedTime() const
{
    if (bIsRunning)
    {
        FTimespan Elapsed = FDateTime::UtcNow() - StartTime;
        return Elapsed.GetTotalMilliseconds();
    }
    return 0.0f;
}

float UExecutionTimer::GetReset()
{
    if (bIsRunning)
    {
        FTimespan Elapsed = FDateTime::UtcNow() - StartTime;
        StartTime = FDateTime::UtcNow();
        return Elapsed.GetTotalMilliseconds();
    }
    return 0.0f;
}