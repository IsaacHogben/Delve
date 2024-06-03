// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGen/ProceduralTerrain.h"
#include "NoiseManager.h"

AProceduralTerrain::AProceduralTerrain()
{
	UE_LOG(LogTemp, Warning, TEXT("ProceduralTerrain Constructor Called"));
	//PrimaryActorTick.bCanEverTick = true;
}

AProceduralTerrain::~AProceduralTerrain()
{
	UE_LOG(LogTemp, Warning, TEXT("ProceduralTerrain Deconstructor called"));
}

// Called when the game starts or when spawned
void AProceduralTerrain::Initialize()
{
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay called in ProceduralTerrain"));
	N = NewObject<UNoiseManager>();
	N->InitializeArray(GenerationNoiseArray);
	// Initialize regions
	BaseRegion = NewObject<UBaseRegion>();
	CliffRegion = NewObject<UCliffRegion>();
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

int AProceduralTerrain::GetBlockIndex(int X, int Y, int Z)
{
	int IndexSize = 262144;
	int r = Z * ChunkSize * ChunkSize + Y * ChunkSize + X;
	if (r >= IndexSize)
		return 0; //get outside chunk!
	return r;
}

bool AProceduralTerrain::IsInLocalRegion(FastNoiseLite* RegionNoise, float RegionSize, float& x, float& y, float& z)
{
	float Region = RegionNoise->GetNoise(x, y, z);

	if (Region < RegionSize)
		return true;
	return false;
}

EBlock AProceduralTerrain::GetBlockFromRegion(ULocalRegion* LocalRegion, ESoilLayer SoilLayer)
{
	switch (SoilLayer) {
	case ESoilLayer::Topsoil:
		return LocalRegion->Topsoil;
	case ESoilLayer::Subsoil:
		return LocalRegion->Subsoil;
	case ESoilLayer::Bedrock:
		return LocalRegion->Bedrock;
	}

	return EBlock::Null;
}

EBlock AProceduralTerrain::GetTerrainLevelOne(float x, float y, float z, EBlock AboveBlock)
{
	ESoilLayer SoilLayer = ESoilLayer::Bedrock;
	ULocalRegion* LocalRegion = BaseRegion;

	if (CliffRegion->IsInRegion(x,y,z))
	{
		float CliffNoiseValue = N->BaseNoise->GetNoise(0.0f, 0.0f, z * 15);
		int QuantizationLevel = FMath::Clamp(FMath::RoundToInt(CliffNoiseValue * 26), 6, 20); // Dynamic quantization
		z = FMath::FloorToInt(z / QuantizationLevel) * QuantizationLevel + 2;
		LocalRegion = CliffRegion;
	}

	const auto SurfaceValue = 0;
	const auto Value = N->BaseNoise->GetNoise(x, y, z);
	const auto UpValue = N->BaseNoise->GetNoise(x, y, z+1);

	const float Density = ZDensityCurve->GetFloatValue(z);
	const float UpDensity = ZDensityCurve->GetFloatValue(z+1);


	if (IsAir(Value, Density))
		return EBlock::Air;
	if (LocalRegion == CliffRegion)
	{
		if (AboveBlock == EBlock::Air)
			return GetBlockFromRegion(CliffRegion, ESoilLayer::Topsoil);
		else if (AboveBlock != EBlock::Null)
			return GetBlockFromRegion(CliffRegion, ESoilLayer::Bedrock);
	}
	else if (UpValue >= UpDensity)
		SoilLayer = ESoilLayer::Topsoil;
	else if (UpValue > Value)
	{
		if (Value >= (Density - 0.0024f))
			SoilLayer = ESoilLayer::Topsoil;
		else if (Value >= (Density - 0.019f))
			SoilLayer = ESoilLayer::Subsoil;
	}

	return GetBlockFromRegion(LocalRegion, SoilLayer);
}


//Generates first layer terrain and returns FBulkBlockUpdate for additional levels of modification.
TArray<FCachedBlockUpdate> AProceduralTerrain::GetGeneratedChunk(FVector ChunkPosition, FIntVector ChunkVectorPosition, TArray<EBlock>& BlockArray, bool& IsChunkEmpty)
{
	TArray<FCachedBlockUpdate> BlockUpdates;
	IsChunkEmpty = false; //WILL NEVER TRIGER! TODO
	EBlock Block;
	EBlock AboveBlock = EBlock::Null;
	for (int x = 0; x < ChunkSize; ++x)
	{
		for (int y = 0; y < ChunkSize; ++y)
		{
			for (int z = ChunkSize; z >= 0 ; --z)
			{
				if (AboveBlock == EBlock::Null)
					AboveBlock = GetTerrainLevelOne(x + ChunkPosition.X, y + ChunkPosition.Y, z + ChunkPosition.Z + 1, EBlock::Null);
				Block = GetTerrainLevelOne(x + ChunkPosition.X, y + ChunkPosition.Y, z + ChunkPosition.Z, AboveBlock);
				AboveBlock = Block;
				//UE_LOG(LogTemp, Warning, TEXT("block %d"), Block);
				BlockArray[GetBlockIndex(x, y, z)] = Block;
				if (Block != EBlock::Air)
					IsChunkEmpty = false;
				if (Block == EBlock::Grass)
				{
					if (FMath::RandRange(0, 512) == 0)
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

bool AProceduralTerrain::IsSurfaceBlock(float UpValue, float Density)
{
	if (IsAir(UpValue, Density))
		return true;
	return false;
}

bool AProceduralTerrain::IsAir(float Value, float Density)
{
	if (Value >= Density)
		return true;
	return false;
}

void AProceduralTerrain::AddReferencelessDecorations(TArray<EBlock>& BlockArray, FastNoiseLite* Noise, TArray<FCachedBlockUpdate>& BlockUpdates)
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

void AProceduralTerrain::MakeTestShape(TArray<FCachedBlockUpdate>& BlockUpdates, int x, int y, int z)
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

void AProceduralTerrain::MakeTestTree(TArray<FCachedBlockUpdate>& BlockUpdates, int height, int x, int y, int z)
{
	//BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y, z), EBlock::Stone));
	//int r = 44;
	for (int i = 1; i < height; i++)
	{
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x, y, z + i), EBlock::Dirt));
	}
	AddSphere(BlockUpdates, 6, x, y, z + height, EBlock::Leaves);
}

void AProceduralTerrain::AddCylinder(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int height, int centerX, int centerY, int baseZ, EBlock blockType)
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

void AProceduralTerrain::AddSphere(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int centerX, int centerY, int centerZ, EBlock blockType)
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