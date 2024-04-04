// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkManager.h"
#include "ChunkRenderDistance.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "../Utils/Enums.h"
#include "../Utils/FastNoiseLite.h"
#include "../Utils/ChunkMeshData.h"

#include <chrono>

class UProceduralMeshComponent;
class AChunkManager; // Forward declaration of AChunkManager

class ChunkClass
{
public:
	ChunkClass();
	~ChunkClass();

	struct FMask
	{
		EBlock Block;
		int Normal;
	};
	
	void BeginPlay();
	void RenderDistanceUpdate(const FVector& Position, int RenderDistance);
	AChunkManager* ChunkManager;
	UProceduralMeshComponent* Mesh;
	//Variables
	UPROPERTY(EditAnywhere, Category = "Chunk")
	int ChunkSize = 32;
	int BlockSize = 50;
	int WorldScale = 50;
	//Represents the chunks position in the block array as multiples of 32
	FVector ChunkPosition;
	//Represents the chunks position vector as whole number;
	FIntVector ChunkVector;

	UPROPERTY(EditAnywhere, Category = "Chunk")
	int Lod = 1;
	TObjectPtr<UMaterialInterface> Material;
	UPROPERTY(EditAnywhere, Category = "Chunk")
	float Frequency = 0.007;

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	void ModifyVoxel(const FIntVector Position, const EBlock Block);

protected:
	// Called when the game starts or when spawned
	

	void Setup();

	void StartAsyncChunkGen();
	void StartAsyncChunkUpdate(const FVector& Position, int RenderDistance);

	void ModifyVoxelData(const FIntVector Position, const EBlock Block);
	void GenerateBlocksFromNoise(FVector Position);

	FastNoiseLite* Noise;
	FChunkMeshData MeshData;
	int VertexCount = 0;

private:
	
	TArray<EBlock> Blocks;
	bool IsChunkEmpty = true;
	//bool ThreadSafe = false;

	int GetBlockIndex(int X, int Y, int Z) const;
	EBlock GetBlock(FIntVector Index, bool checkOutsideChunk);

	//FGraphEventRef FirstTask;
	void GenerateChunkAsync();
	void GenerateChunkAsyncComplete();

	void UpdateChunkAsync(const FVector& Position, int RenderDistance);
	void UpdateChunkAsyncComplete();
	
	void GenerateMesh();
	void CreateQuad(FMask Mask, FIntVector AxisMask, int Width, int Height, FIntVector V1, FIntVector V2, FIntVector V3, FIntVector V4);
	bool CompareMask(const FMask M1, const FMask M2) const;
	void ApplyMesh();
	void ClearMeshData();

	int GetTextureIndex(EBlock Block, FVector Normal) const;
	void PostStats(std::chrono::high_resolution_clock::time_point StartTime);
};
