// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/VectorFunctionUtils.h"

VectorFunctionUtils::VectorFunctionUtils()
{
}

VectorFunctionUtils::~VectorFunctionUtils()
{
}

FIntVector VectorFunctionUtils::FVectorToFIntVector(const FVector& InVector)
{
	// Round each component of the FVector to the nearest integer
	int32 X = FMath::RoundToInt(InVector.X);
	int32 Y = FMath::RoundToInt(InVector.Y);
	int32 Z = FMath::RoundToInt(InVector.Z);

	return FIntVector(X, Y, Z);
}