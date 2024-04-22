// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
static class VectorFunctionUtils
{
public:
	VectorFunctionUtils();
	~VectorFunctionUtils();

	static FIntVector FVectorToFIntVector(const FVector& InVector);
};
