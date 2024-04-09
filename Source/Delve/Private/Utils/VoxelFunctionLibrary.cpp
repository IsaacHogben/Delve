// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelFunctionLibrary.h"

FIntVector UVoxelFunctionLibrary::WorldToBlockPosition(const FVector& Position)
{
	return FIntVector(Position) / 100;
}

FIntVector UVoxelFunctionLibrary::WorldToLocalBlockPosition(const FVector& Position, const int Size)
{
	const auto ChunkPos = WorldToChunkPosition(Position, Size);
	
	auto Result = WorldToBlockPosition(Position) - ChunkPos * Size;

	// Negative Normalization
	if (ChunkPos.X < 0) Result.X--;
	if (ChunkPos.Y < 0) Result.Y--;
	if (ChunkPos.Z < 0) Result.Z--;

	return Result;
}

FIntVector UVoxelFunctionLibrary::WorldToChunkPosition(const FVector& Position, const int Size)
{
	FIntVector Result;

	const int Factor = Size * 100;
	const auto IntPosition = FIntVector(Position);

	if (IntPosition.X < 0) Result.X = (int)(Position.X / Factor) - 1;
	else Result.X = (int)(Position.X / Factor);

	if (IntPosition.Y < 0) Result.Y = (int)(Position.Y / Factor) - 1;
	else Result.Y = (int)(Position.Y / Factor);

	if (IntPosition.Z < 0) Result.Z = (int)(Position.Z / Factor) - 1;
	else Result.Z = (int)(Position.Z / Factor);
	
	return Result;
}

bool UVoxelFunctionLibrary::IntVectorCompare(const FIntVector& Position1, const FIntVector& Position2)
{
	if (Position1.X == Position2.X && Position1.Y == Position2.Y && Position1.Z == Position2.Z)
		return 1;
	return 0;
}

FVector UVoxelFunctionLibrary::VectorRoundingAdjustment(const FVector& Vector)
{
	FVector Adjusted = FVector::Zero();
	for (int i = 0; i < 3; i++)
	{
		if (Vector[i] < 0)
			Adjusted[i] = Vector[i] - 1;
		else
			Adjusted[i] = Vector[i];
	}
	return Adjusted;
}
