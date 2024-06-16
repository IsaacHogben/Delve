// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkInclude.h"
#include "../Utils/FastNoiseLite.h"
#include "../Utils/ChunkStructs.h"

#include "GenClasses/Truce/BaseRegion.h"
#include "GenClasses/Truce/CliffRegion.h"

#include "ProceduralTerrain.generated.h"

class UNoiseManager;
struct FCachedBlockUpdate;
struct FFastNoise;
/**
 * 
 */

UCLASS()
class AProceduralTerrain : public AActor
{
	GENERATED_BODY()

public:
	AProceduralTerrain();
	~AProceduralTerrain();
	void Initialize();
	UPROPERTY()
	UNoiseManager* N;

	UPROPERTY(EditAnywhere, Category = "Generation Settings")
	TArray<FFastNoise> GenerationNoiseArray;

	// The reference to the curve asset
	UPROPERTY(EditAnywhere, Category = "Curves")
	UCurveFloat* ZDensityCurve;
	UPROPERTY(EditAnywhere, Category = "Curves")
	UCurveFloat* WorldEdgeDensityCurve;

	FIntVector2 WorldCenter = FIntVector2(0, 0);
	int ChunkSize = 64;
	int GetBlockIndex(int X, int Y, int Z);
	
	TArray<FCachedBlockUpdate> GetGeneratedChunk(FVector ChunkPosition, FIntVector ChunkVectorPosition, TArray<EBlock>& BlockArray, bool& isChunkEmpty);

protected:


private:
	EBlock GetTerrainLevelOne(float x, float y, float z, EBlock AboveBlock);
	float GetNoiseLevelOne(float x, float y, float z);

	bool IsInLocalRegion(FastNoiseLite* Region, float RegionSize, float& x, float& y, float& z);
	EBlock GetBlockFromRegion(ULocalRegion* LocalRegion, ESoilLayer SoilLayer);

	bool IsSurfaceBlock(float AboveValue, float Density);
	bool IsAir(float Value, float Density);

	void AddReferencelessDecorations(TArray<EBlock>& BlockArray, FastNoiseLite* Noise, TArray<FCachedBlockUpdate>&
		BlockUpdates);
	void MakeTestShape(TArray<FCachedBlockUpdate>& BlockUpdates, int x, int y, int z);
	void MakeTestTree(TArray<FCachedBlockUpdate>& BlockUpdates, int x, int y, int z);
	void AddCylinder(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int height, int centerX, int centerY, int baseZ, EBlock blockType);
	void AddSphere(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int centerX, int centerY, int centerZ, EBlock blockType);

	//Local Regions
	UPROPERTY()
	class UBaseRegion* BaseRegion;
	UPROPERTY()
	class UCliffRegion* CliffRegion;
	UPROPERTY()
	class UTopRegion* TopRegion;

};

int QuantizeCoordinate(int value, int quantizationStep);

float GetQuantizedNoise(int x, int y, int z, FastNoiseLite* Noise);
