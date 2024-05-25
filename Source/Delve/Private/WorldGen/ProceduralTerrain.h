// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkInclude.h"
#include "../Utils/FastNoiseLite.h"
#include "../Utils/ChunkStructs.h"

struct FCachedBlockUpdate;
/**
 * 
 */
class ProceduralTerrain
{
public:
	ProceduralTerrain();
	~ProceduralTerrain();

	int ChunkSize = 64;

	static int GetBlockIndex(int X, int Y, int Z);

	static EBlock GetTerrainBlock(float x, float y, float z, FastNoiseLite* Noise);
	static TArray<FCachedBlockUpdate> GetGeneratedChunk(FVector ChunkPosition, FIntVector ChunkVectorPosition, int ChunkSize, TArray<EBlock>& BlockArray, FastNoiseLite* Noise, bool& isChunkEmpty);
	
private:
	bool IsSurfaceBlock(float AboveValue, float Density);
	bool IsAir(float Value, float Density);

	static void AddReferencelessDecorations(TArray<EBlock>& BlockArray, FastNoiseLite* Noise, TArray<FCachedBlockUpdate>&
		BlockUpdates);
	static void MakeTestShape(TArray<FCachedBlockUpdate>& BlockUpdates, int x, int y, int z);
	static void MakeTestTree(TArray<FCachedBlockUpdate>& BlockUpdates, int height, int x, int y, int z);
	void AddCylinder(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int height, int centerX, int centerY, int baseZ, EBlock blockType);
	static void AddSphere(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int centerX, int centerY, int centerZ, EBlock blockType);
};
