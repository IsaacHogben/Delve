// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkInclude.h"
#include "../Utils/FastNoiseLite.h"
#include "../Utils/ChunkStructs.h"

class UNoiseManager;
struct FCachedBlockUpdate;
struct FFastNoise;
/**
 * 
 */
class ProceduralTerrain
{
public:
	ProceduralTerrain();
	~ProceduralTerrain();
	UPROPERTY()
	UNoiseManager* N;

	int ChunkSize = 64;

	int GetBlockIndex(int X, int Y, int Z);

	
	TArray<FCachedBlockUpdate> GetGeneratedChunk(FVector ChunkPosition, FIntVector ChunkVectorPosition, TArray<EBlock>& BlockArray, bool& isChunkEmpty);
	
private:
	EBlock GetTerrainLevelOne(float x, float y, float z);
	float GetNoiseLevelOne(float x, float y, float z);

	bool IsSurfaceBlock(float AboveValue, float Density);
	bool IsAir(float Value, float Density);

	void AddReferencelessDecorations(TArray<EBlock>& BlockArray, FastNoiseLite* Noise, TArray<FCachedBlockUpdate>&
		BlockUpdates);
	void MakeTestShape(TArray<FCachedBlockUpdate>& BlockUpdates, int x, int y, int z);
	void MakeTestTree(TArray<FCachedBlockUpdate>& BlockUpdates, int height, int x, int y, int z);
	void AddCylinder(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int height, int centerX, int centerY, int baseZ, EBlock blockType);
	void AddSphere(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int centerX, int centerY, int centerZ, EBlock blockType);
};

int QuantizeCoordinate(int value, int quantizationStep);

float GetQuantizedNoise(int x, int y, int z, FastNoiseLite* Noise);
