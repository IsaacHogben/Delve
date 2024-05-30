// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGen/ProceduralTerrain.h"
#include "NoiseManager.h"

UProceduralTerrain::UProceduralTerrain()
{
}

UProceduralTerrain::~UProceduralTerrain()
{
	UE_LOG(LogTemp, Warning, TEXT("ProceduralTerrain Deconstructor called"));
}

int QuantizeCoordinate(int value, int quantizationStep)
{
	if (quantizationStep == 0)
		return value;
	return (value / quantizationStep) * quantizationStep;
}

float GetQuantizedNoise(int x, int y, int z, FastNoiseLite* Noise)
{

	int QuantizationStep = 3;
	// Quantize the input coordinates
	float quantizedX = QuantizeCoordinate(x, QuantizationStep);
	float quantizedY = QuantizeCoordinate(y, QuantizationStep);
	float quantizedZ = QuantizeCoordinate(z, QuantizationStep);

	// Get the noise value using quantized coordinates
	return Noise->GetNoise(float(x), float(y), quantizedZ);
}

int UProceduralTerrain::GetBlockIndex(int X, int Y, int Z)
{
	int IndexSize = 262144;
	int r = Z * ChunkSize * ChunkSize + Y * ChunkSize + X;
	if (r >= IndexSize)
		return 0; //get outside chunk!
	return r;
}

float UProceduralTerrain::GetNoiseLevelOne(float x, float y, float z)
{
	float CliffRegion = N->CliffNoise->GetNoise(x, y, z);

	if (CliffRegion < -0.88)
		z = QuantizeCoordinate(z, (((N->BaseNoise->GetNoise(0.0f, 0.0f, z)/2) + 1) * 8) - 3);//extra math not doing much atm

	return N->BaseNoise->GetNoise(x, y, z);
}

EBlock UProceduralTerrain::GetTerrainLevelOne(float x, float y, float z)
{
	//NoiseTest
	float CliffRegion = N->CliffNoise->GetNoise(x, y, z);

	const auto SurfaceValue = 0;
	const auto Value = GetNoiseLevelOne(x, y, z);
	const auto UpValue = GetNoiseLevelOne(x, y, z + 1);

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
TArray<FCachedBlockUpdate> UProceduralTerrain::GetGeneratedChunk(FVector ChunkPosition, FIntVector ChunkVectorPosition, TArray<EBlock>& BlockArray, bool& IsChunkEmpty)
{
	TArray<FCachedBlockUpdate> BlockUpdates;
	IsChunkEmpty = false; //WILL NEVER TRIGER! TODO
	EBlock Block;
	for (int x = 0; x < ChunkSize; ++x)
	{
		for (int y = 0; y < ChunkSize; ++y)
		{
			for (int z = 0; z < ChunkSize ; ++z)
			{
				Block = GetTerrainLevelOne(x + ChunkPosition.X, y + ChunkPosition.Y, z + ChunkPosition.Z);
				BlockArray[GetBlockIndex(x, y, z)] = Block;
				if (Block != EBlock::Air)
					IsChunkEmpty = false;
				if (Block == EBlock::Grass)
				{
					if (FMath::RandRange(0, 256) == 0)
						MakeTestTree(BlockUpdates, 15, x, y, z);
				}
			}
		}
	}

	//AddReferencelessDecorations(BlockArray, Noise, BlockUpdates);
	//MakeTestShape(BlockUpdates, -1,-1,-1);
	//MakeTestShape(BlockUpdates, 0,0,0);
	return BlockUpdates;
}

bool UProceduralTerrain::IsSurfaceBlock(float UpValue, float Density)
{
	if (IsAir(UpValue, Density))
		return true;
	return false;
}

bool UProceduralTerrain::IsAir(float Value, float Density)
{
	if (Value >= Density)
		return true;
	return false;
}

void UProceduralTerrain::AddReferencelessDecorations(TArray<EBlock>& BlockArray, FastNoiseLite* Noise, TArray<FCachedBlockUpdate>& BlockUpdates)
{
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

void UProceduralTerrain::MakeTestShape(TArray<FCachedBlockUpdate>& BlockUpdates, int x, int y, int z)
{
	//BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y, z), EBlock::Stone));
	//int r = 44;
	for (int i = 1; i < 64; i++)
	{ 	
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x + i, y, z), EBlock::Stone));
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x, y + i, z), EBlock::Stone));
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x, y, z + i), EBlock::Stone));
	}
	x = 63;
	y = 63;
	z = 63;
	for (int i = 1; i < 64; i++)
	{
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x - i, y, z), EBlock::Stone));
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x, y - i, z), EBlock::Stone));
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x, y, z - i), EBlock::Stone));
	}
}

void UProceduralTerrain::MakeTestTree(TArray<FCachedBlockUpdate>& BlockUpdates, int height, int x, int y, int z)
{
	//BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y, z), EBlock::Stone));
	//int r = 44;
	for (int i = 1; i < height; i++)
	{
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x, y, z + i), EBlock::Dirt));
	}
	AddSphere(BlockUpdates, 6, x, y, z + height, EBlock::Leaves);
}

void UProceduralTerrain::AddCylinder(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int height, int centerX, int centerY, int baseZ, EBlock blockType)
{
	for (int z = 0; z < height; ++z)
	{
		for (int x = -radius; x <= radius; ++x)
		{
			for (int y = -radius; y <= radius; ++y)
			{
				if (x * x + y * y <= radius * radius)
				{
					BlockUpdates.Add(FCachedBlockUpdate( FIntVector(centerX + x, centerY + y, baseZ + z), blockType));
				}
			}
		}
	}
}

void UProceduralTerrain::AddSphere(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int centerX, int centerY, int centerZ, EBlock blockType)
{
	for (int x = -radius; x <= radius; ++x)
	{
		for (int y = -radius; y <= radius; ++y)
		{
			for (int z = -radius; z <= radius; ++z)
			{
				if (x * x + y * y + z * z <= radius * radius)
				{
					BlockUpdates.Add(FCachedBlockUpdate(FIntVector(centerX + x, centerY + y, centerZ + z), blockType));
				}
			}
		}
	}
}