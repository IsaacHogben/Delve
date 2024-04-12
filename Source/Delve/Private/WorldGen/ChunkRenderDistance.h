// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utils/Enums.h"

/**
 * 
 */
class ChunkRenderDistance
{
public:
	struct ChunkSpawnData
	{
		int Lod;
		FIntVector Position;
	};

	ChunkRenderDistance(int RenderDistance);
	~ChunkRenderDistance();

	TArray<ChunkSpawnData> CalculateRenderSphere();
	int CalculateLod(float Distance);
	static float FVectorDistance(const FVector& Vector1, const FVector& Vector2);

private:
	int MaxRenderDistance;
	int LodRenderDistance;
	TArray<int8> LodArray = {1, 1, 1, 1, 2, 2, 4, 8, 16, 32, 64};
	//int DrawDistance = 5;
	//TArray<TArray<FIntVector>> RenderHemisphere;
};
