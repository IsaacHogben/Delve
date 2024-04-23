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

static float FIntVectorDistance(const FIntVector& Vector1, const FIntVector& Vector2)
{
	//UE_LOG(LogTemp, Warning, TEXT("Chunk pos, %f,%f,%f"), Vector2.X, Vector2.Y, Vector2.Z);
	//UE_LOG(LogTemp, Warning, TEXT("Player pos %f,%f,%f"), Vector1.X, Vector1.Y, Vector1.Z);
	// Calculate the components of the vector between the two points
	float DeltaX = Vector1.X - Vector2.X;
	float DeltaY = Vector1.Y - Vector2.Y;
	float DeltaZ = Vector1.Z - Vector2.Z;

	// Calculate the 3D distance between the two points using the Pythagorean theorem
	float Distance = FMath::Sqrt(DeltaX * DeltaX + DeltaY * DeltaY + DeltaZ * DeltaZ);

	return Distance;
}