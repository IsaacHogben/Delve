// Fill out your copyright notice in the Description page of Project Settings.
#include "ChunkRenderDistance.h"

ChunkRenderDistance::ChunkRenderDistance(int RenderDistance)
{
	MaxRenderDistance = RenderDistance;
}

ChunkRenderDistance::~ChunkRenderDistance()
{
}

//Defines the initial play zone for the chunk manager
TArray<ChunkRenderDistance::ChunkSpawnData> ChunkRenderDistance::CalculateRenderSphere()
{
	TArray<ChunkSpawnData> chunkSpawnData;

	FVector PlayerPosition = FVector(0, 0, 0);
	TArray<ChunkSpawnData> dataArray;

	for (int z = -MaxRenderDistance; z <= 0; z++)
	{
		for (int x = -MaxRenderDistance; x <= MaxRenderDistance; x++)
		{
			for (int y = -MaxRenderDistance; y <= MaxRenderDistance; y++)
			{
				float distance = FMath::Sqrt(FVector::DistSquared(PlayerPosition, FVector(x, y, z)));
				if (distance < MaxRenderDistance)
				{
					ChunkSpawnData data;
					data.Position = FIntVector(x, y, z);
					data.Lod = CalculateLod(distance);
					dataArray.Add(data);
					//return dataArray;//temptest
				}
			}
		}
	}
	return dataArray;
}

//Calculates Lod for a chunk, based on a chunks vector distance from player
int ChunkRenderDistance::CalculateLod(float Distance)
{
	int lod = static_cast<int>(Distance) / (MaxRenderDistance / 6);
	if (lod >= 6)
		lod = 5;
	//UE_LOG(LogTemp, Warning, TEXT("%d | %f"), lod, Distance);
	return LodArray[lod];//Must return a vlaue from 0 to 5 as these are our available LODS
}
