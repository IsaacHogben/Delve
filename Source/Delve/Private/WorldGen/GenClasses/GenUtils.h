// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class GenUtils
{
public:
	GenUtils();
	~GenUtils();

    static float normalizeValue(double value, double minValue, double maxValue)
    {
        return (value - minValue) / (maxValue - minValue);
    }
};
