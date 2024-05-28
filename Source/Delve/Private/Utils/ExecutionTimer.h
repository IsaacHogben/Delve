// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "ExecutionTimer.generated.h"

UCLASS()
class UExecutionTimer : public UObject
{
    GENERATED_BODY()

public:
    UExecutionTimer();

    // Start the timer
    void Start();

    // Stop the timer and return the elapsed time in milliseconds
    float Stop();

    // Get the elapsed time without stopping the timer
    float GetElapsedTime() const;

    // Get the elapsed time and reset the timer
    float GetReset();

private:
    FDateTime StartTime;
    FDateTime EndTime;
    bool bIsRunning;
};