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

	

	//TArray<int8> LodDistanceArray;
private:
	int MaxRenderDistance;
	TArray<int8> LodArray = { 1, 2, 4, 8, 16, 32 };
	//int DrawDistance = 5;
	//TArray<TArray<FIntVector>> RenderHemisphere;
};
