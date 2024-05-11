// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkInclude.h"
#include "../Utils/FastNoiseLite.h"
#include "../Utils/ChunkStructs.h"

struct FBlockUpdate;
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
	static TArray<FBlockUpdate> GetGeneratedChunk(FVector ChunkPosition, FIntVector ChunkVectorPosition, int ChunkSize, TArray<EBlock>& BlockArray, FastNoiseLite* Noise, bool& isChunkEmpty);
	
private:
	bool IsSurfaceBlock(float AboveValue, float Density);
	bool IsAir(float Value, float Density);

	static void AddReferencelessDecorations(TArray<EBlock>& BlockArray, FastNoiseLite* Noise, TArray<FBlockUpdate>&
		BlockUpdates);
	static void UpdateDispatchInfoForBlockUpdates(TArray<FBlockUpdate>& BlockUpdates, FIntVector ChunkVectorPosition);
	static void MakeTestShape(TArray<FBlockUpdate>& BlockUpdates, int x, int y, int z);
};
