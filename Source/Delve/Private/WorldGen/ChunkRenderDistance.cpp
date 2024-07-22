// Fill out your copyright notice in the Description page of Project Settings.
#include "ChunkRenderDistance.h"
#include "../Utils/ChunkStructs.h"

ChunkRenderDistance::ChunkRenderDistance(int RenderDistance)
{
	MaxRenderDistance = RenderDistance;
	if (RenderDistance > LodArray.Num())
		LodRenderDistance = RenderDistance;
	else LodRenderDistance = LodArray.Num();
}

ChunkRenderDistance::~ChunkRenderDistance()
{
}

//Defines the initial play zone for the chunk manager
TArray<FChunkData> ChunkRenderDistance::CalculateRenderSphere()
{
	TArray<FChunkData> chunkSpawnData;

	FVector PlayerPosition = FVector(0, 0, 0);
	TArray<FChunkData> dataArray;

	for (int z = -MaxRenderDistance; z <= MaxRenderDistance; z++)
	{
		for (int x = -MaxRenderDistance; x <= MaxRenderDistance; x++)
		{
			for (int y = -MaxRenderDistance; y <= MaxRenderDistance; y++)
			{
				float distance = FVectorDistance(PlayerPosition, FVector(x, y, z));
				if (distance <= MaxRenderDistance)
				{
					FChunkData data;
					data.Position = FIntVector(x, y, z);
					data.Lod = CalculateLod(distance);
					dataArray.Add(data);
				}
			}
		}
	}
	dataArray.Sort();
	return dataArray;
}

TArray<FChunkData> ChunkRenderDistance::CalculateRenderDisk()
{
	TArray<FChunkData> chunkSpawnData;

	FVector PlayerPosition = FVector(0, 0, 0);
	TArray<FChunkData> dataArray;

	for (int z = 5; z >= -7; z--) //-7 base
	{
		for (int x = -MaxRenderDistance; x <= MaxRenderDistance; x++)
		{
			for (int y = -MaxRenderDistance; y <= MaxRenderDistance; y++)
			{
				float distance = FVectorDistance(FVector(PlayerPosition.X, PlayerPosition.Y, 0), FVector(x, y, 0));
				if (distance <= MaxRenderDistance)
				{
					FChunkData data;
					data.Position = FIntVector(x, y, z);
					data.Lod = 1;
					dataArray.Add(data);
				}
			}
		}
	}
	dataArray.Sort();
	return dataArray;
}

//Calculates Lod for a chunk, based on a chunks vector distance from player
int ChunkRenderDistance::CalculateLod(float Distance)
{
	
	int lod = static_cast<int>(Distance) / (LodRenderDistance / LodArray.Num());
	if (lod >= LodArray.Num())
		lod = LodArray.Num() - 1;
	//UE_LOG(LogTemp, Warning, TEXT("Lod Calculated at %d | %f"), lod, Distance);
	return LodArray[lod];
}

static float FVectorDistance(const FVector& Vector1, const FVector& Vector2)
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


