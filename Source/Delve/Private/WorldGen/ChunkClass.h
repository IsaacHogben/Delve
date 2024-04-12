// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkInclude.h"
#include "ChunkRenderDistance.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

#include "Misc/DateTime.h"
#include "../Utils/Enums.h"
#include "../Utils/FastNoiseLite.h"
#include "../Utils/ChunkMeshData.h"
#include <chrono>

#include "ChunkClass.generated.h"

class UProceduralMeshComponent;
class AChunkManager; // Forward declaration of AChunkManager

USTRUCT()
struct FMask
{
	GENERATED_BODY()

	UPROPERTY()
	EBlock Block;

	UPROPERTY()
	int Normal;
};

UCLASS()
class UChunkClass : public UObject
{
	GENERATED_BODY()

public:
	UChunkClass();
	~UChunkClass();
	
	void BeginPlay();
	void RenderDistanceUpdate(const FVector& Position, int RenderDistance);
	UPROPERTY()
	AChunkManager* ChunkManager;
	UPROPERTY()
	UProceduralMeshComponent* Mesh;
	//Variables
	UPROPERTY(EditAnywhere, Category = "Chunk")
	int ChunkSize = 64;
	int BlockSize = 50;
	int WorldScale = 50;
	//Represents the chunks position in the block array as multiples of 32
	UPROPERTY()
	FVector ChunkPosition;
	//Represents the chunks position vector as whole number;
	UPROPERTY()
	FIntVector ChunkVector;

	UPROPERTY(EditAnywhere, Category = "Chunk")
	int Lod = 1;
	TObjectPtr<UMaterialInterface> Material;
	UPROPERTY(EditAnywhere, Category = "Chunk")
	float Frequency = 0.005;

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	void ModifyVoxel(const FIntVector Position, const EBlock Block);

protected:
	// Called when the game starts or when spawned
	

	void Setup();

	void StartAsyncChunkGen(const FVector& PlayerPosition);
	void StartAsyncChunkUpdate(const FVector& Position, int RenderDistance);

	void ModifyVoxelData(const FIntVector Position, const EBlock Block);
	void GenerateBlocksFromNoise(FVector Position);
	FastNoiseLite* Noise;

	UPROPERTY()
	int VertexCount = 0;

private:
	TArray<EBlock> Blocks;
	UPROPERTY()
	TArray<FIntVector> PerspectiveMask;
	FChunkMeshData* MeshData;
	bool IsChunkEmpty = true;


	int GetBlockIndex(int X, int Y, int Z) const;
	EBlock GetBlock(FIntVector Index, bool checkOutsideChunk);

	void GenerateChunkAsync(const FVector& PlayerPosition);
	void GenerateChunkAsyncComplete();

	void UpdateChunkAsync(const FVector& Position, int RenderDistance);
	void UpdateChunkAsyncComplete();
	
	void GenerateMesh(const FVector& PlayerPosition);
	void CreateQuad(FMask Mask, FIntVector AxisMask, int Width, int Height, FVector V1, FVector V2, FVector V3, FVector V4);
	bool CompareMask(const FMask M1, const FMask M2) const;
	TArray<FIntVector> CalculatePerspectiveMask(FVector PlayerPosition);
	bool CompareNormalMask(FIntVector Normal);
	void ApplyMesh();
	void ClearMeshData();

	int GetTextureIndex(EBlock Block, FVector Normal) const;
	void PostStats(std::chrono::high_resolution_clock::time_point StartTime);
};
