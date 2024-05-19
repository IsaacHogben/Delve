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
	int IndexSize = 262144;
	int r = Z * ChunkSize * ChunkSize + Y * ChunkSize + X;
	if (r >= IndexSize)
		return 0; //get outside chunk!
	return r;
}

EBlock ProceduralTerrain::GetTerrainBlock(float x, float y, float z, FastNoiseLite* Noise)
{
	
	//if (1) return EBlock::Air;
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
	for (int x = 0; x < ChunkSize; ++x)
	{
		for (int y = 0; y < ChunkSize; ++y)
		{
			for (int z = 0; z < ChunkSize ; ++z)
			{
				Block = ProceduralTerrain::GetTerrainBlock(x + ChunkPosition.X, y + ChunkPosition.Y, z + ChunkPosition.Z, Noise);
				BlockArray[GetBlockIndex(x, y, z)] = Block;
				if (Block != EBlock::Air)
					IsChunkEmpty = false;
			}
		}
	}

	//AddReferencelessDecorations(BlockArray, Noise, BlockUpdates);
	//MakeTestShape(BlockUpdates, -1,-1,-1);
	MakeTestShape(BlockUpdates, 0,0,0);
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
					{
						MakeTestShape(BlockUpdates, x + 1, y, z + 1);
						MakeTestShape(BlockUpdates, x + 1, y + 1, z + 1);
						MakeTestShape(BlockUpdates, x, y+ 1, z + 1);
						MakeTestShape(BlockUpdates, x, y, z + 1);
					}
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
	//int r = 44;
	for (int i = 1; i < 64; i++)
	{ 	
		BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x + i, y, z), EBlock::Stone));
		BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y + i, z), EBlock::Stone));
		BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y, z + i), EBlock::Stone));
	}
	x = 63;
	y = 63;
	z = 63;
	for (int i = 1; i < 64; i++)
	{
		BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x - i, y, z), EBlock::Stone));
		BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y - i, z), EBlock::Stone));
		BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y, z - i), EBlock::Stone));
	}
}
