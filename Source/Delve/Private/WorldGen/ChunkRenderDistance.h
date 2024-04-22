// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utils/Enums.h"
#include "Utils/ChunkStructs.h"

struct FChunkSpawnData;

class ChunkRenderDistance
{
public:

	ChunkRenderDistance(int RenderDistance);
	~ChunkRenderDistance();

	TArray<FChunkSpawnData> CalculateRenderSphere();
	int CalculateLod(float Distance);
	static float FVectorDistance(const FVector& Vector1, const FVector& Vector2);

private:
	int MaxRenderDistance;
	int LodRenderDistance;
	//TArray<int8> LodArray = {1, 1, 1, 1, 1, 1, 2, 2, 2, 4, 4, 8, 16, 32, 64, 64, 64, 64};
	TArray<int8> LodArray = {1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 8, 8, 8, 16, 16, 32};
	//int DrawDistance = 5;
	//TArray<TArray<FIntVector>> RenderHemisphere;
};
