// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGen/ProceduralTerrain.h"

ProceduralTerrain::ProceduralTerrain()
{
}

ProceduralTerrain::~ProceduralTerrain()
{
}

int ProceduralTerrain::GetBlockIndex(int X, int Y, int Z)
{
	int ChunkSize = 64;
	int IndexSize = 287496;
	int r = Z * (ChunkSize + 2) * (ChunkSize + 2) + Y * (ChunkSize + 2) + X;
	if (r >= IndexSize)
		return 0; //get outside chunk!
	return r;
}

EBlock ProceduralTerrain::GetTerrainBlock(float x, float y, float z, FastNoiseLite* Noise)
{
	const auto SurfaceValue = Noise->GetNoise(x, y, 0.0f);
	const auto Value = Noise->GetNoise(x, y, z);
	const auto UpValue = Noise->GetNoise(x, y, z + 1);

	const float Density = 0;

	if (Value >= Density || z + (32 * SurfaceValue) > 0)
	{
		return EBlock::Air;
	}
	else //Not Air
	{
		if (UpValue >= Density)
			return EBlock::Grass;
		if (UpValue > Value)
		{ 
			if (Value >= (Density - 0.0024f))
				return EBlock::Grass;
			if (Value >= (Density - 0.016f))
				return EBlock::Dirt;
		}
		return EBlock::Stone;
	}
}

//Generates first layer terrain and returns FBulkBlockUpdate for additional levels of modification.
TArray<FBlockUpdate> ProceduralTerrain::GetGeneratedChunk(FVector ChunkPosition, FIntVector ChunkVectorPosition, int ChunkSize, TArray<EBlock>& BlockArray, FastNoiseLite* Noise, bool& IsChunkEmpty)
{
	TArray<FBlockUpdate> BlockUpdates;
	IsChunkEmpty = false; //WILL NEVER TRIGER! TODO
	EBlock Block;
	for (int x = 0; x < ChunkSize + 2; ++x)
	{
		for (int y = 0; y < ChunkSize + 2; ++y)
		{
			for (int z = 0; z < ChunkSize + 2; ++z)
			{
				Block = ProceduralTerrain::GetTerrainBlock(x + ChunkPosition.X, y + ChunkPosition.Y, z + ChunkPosition.Z, Noise);
				BlockArray[GetBlockIndex(x, y, z)] = Block;
				if (Block != EBlock::Air)
					IsChunkEmpty = false;
			}
		}
	}
	if (IsChunkEmpty)
		return BlockUpdates;
	AddReferencelessDecorations(BlockArray, Noise, BlockUpdates);
	//MakeTestShape(BlockUpdates, 0,0,0);
	UpdateDispatchInfoForBlockUpdates(BlockUpdates, ChunkVectorPosition);
	return BlockUpdates;
}

bool ProceduralTerrain::IsSurfaceBlock(float UpValue, float Density)
{
	if (IsAir(UpValue, Density))
		return true;
	return false;
}

bool ProceduralTerrain::IsAir(float Value, float Density)
{
	if (Value >= Density)
		return true;
	return false;
}

void ProceduralTerrain::AddReferencelessDecorations(TArray<EBlock>& BlockArray, FastNoiseLite* Noise, TArray<FBlockUpdate>& BlockUpdates)
{
	int ChunkSize = 64;
	EBlock Block;
	for (int x = 0; x < ChunkSize + 2; ++x)
	{
		for (int y = 0; y < ChunkSize + 2; ++y)
		{
			for (int z = 0; z < ChunkSize + 2; ++z)
			{
				 Block = BlockArray[GetBlockIndex(x, y, z)];
				//check for condition and make changes
				if (Block == EBlock::Grass && BlockArray[GetBlockIndex(x, y, z + 1)] == EBlock::Air)
				{
					if (FMath::RandRange(0, 124) == 0)
						MakeTestShape(BlockUpdates, x, y, z + 1);
				}
			}
		}
	}
}

void ProceduralTerrain::UpdateDispatchInfoForBlockUpdates(TArray<FBlockUpdate>& BlockUpdates, FIntVector ChunkVectorPosition)
{
	for (int i = 0; i < BlockUpdates.Num(); i++)
	{
		BlockUpdates[i].DispatchChunk = ChunkVectorPosition;
		BlockUpdates[i].TargetChunk = ChunkVectorPosition;
	}
}

void ProceduralTerrain::MakeTestShape(TArray<FBlockUpdate>& BlockUpdates, int x, int y, int z)
{
	//BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y, z), EBlock::Stone));
	int r = FMath::RandRange(7, 25);
	for (int i = 0; i < r; i++)
	{ 	
		if (i == (r - 1))
		{
			BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y, z + i), EBlock::Grass));
			BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x + 1, y, z + i - 1), EBlock::Grass));
			BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x - 1, y, z + i - 1), EBlock::Grass));
			BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y + 1, z + i - 1), EBlock::Grass));
			BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y - 1, z + i - 1), EBlock::Grass));
			BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x + 2, y, z + i - 2), EBlock::Grass));
			BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x - 2, y, z + i - 2), EBlock::Grass));
			BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y + 2, z + i - 2), EBlock::Grass));
			BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y - 2, z + i - 2), EBlock::Grass));
		}
		else
			BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y, z + i), EBlock::Dirt));
	}
}
