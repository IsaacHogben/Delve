// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utils/Enums.h"
#include "Utils/ChunkStructs.h"

struct FChunkData;

class ChunkRenderDistance
{
public:

	ChunkRenderDistance(int RenderDistance);
	~ChunkRenderDistance();

	TArray<FChunkData> CalculateRenderSphere();
	int CalculateLod(float Distance);


private:
	int MaxRenderDistance;
	int LodRenderDistance;
	//TArray<int8> LodArray = {1, 1, 1, 1, 2, 2, 2, 4, 4, 8, 16, 32, 64, 64, 64, 64};
	TArray<int8> LodArray = {1, 1, 1, 1, 1, 1, 1, 1};
	//int DrawDistance = 5;
	//TArray<TArray<FIntVector>> RenderHemisphere;
};

static float FVectorDistance(const FVector& Vector1, const FVector& Vector2);
static float FIntVectorDistance(const FIntVector& Vector1, const FIntVector& Vector2);
