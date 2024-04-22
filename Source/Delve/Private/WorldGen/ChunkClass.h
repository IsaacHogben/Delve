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
#include "../Utils/VectorFunctionUtils.h"
#include <chrono>

#include "ChunkClass.generated.h"

class UProceduralMeshComponent;
class AChunkManager; // Forward declaration of AChunkManager

USTRUCT()
struct FMask
{
	GENERATED_BODY()

	//UPROPERTY()
	EBlock Block;

	//UPROPERTY()
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
	void RenderDistanceUpdate(const FVector& Position, int RenderDistance, const FIntVector& Direction);
	UPROPERTY()
	AChunkManager* ChunkManager;
	UPROPERTY()
	UProceduralMeshComponent* Mesh;
	//Variables
	UPROPERTY(EditAnywhere, Category = "Chunk")
	int ChunkSize = 32;
	int BlockSize = 50;
	int WorldScale = 50;
	
	UPROPERTY()// Represents the chunks position in the world
	FVector ChunkWorldPosition;
	
	UPROPERTY()// Represents the chunks position vector as whole number;
	FIntVector ChunkVectorPosition;

	UPROPERTY()// Represents the center of the latest chunk render sphere. Used as reference around when moving chunks
	FIntVector CentralRenderChunkVector;

	UPROPERTY(EditAnywhere, Category = "Chunk")
	int Lod = 1;
	TObjectPtr<UMaterialInterface> Material;
	UPROPERTY(EditAnywhere, Category = "Chunk")
	float Frequency = 0.03;

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	void ModifyVoxel(const FIntVector Position, const EBlock Block);

	void UpdateChunkPosition(FIntVector ChunkVectorPosition);

protected:
	// Called when the game starts or when spawned
	
	void Setup();

	void StartAsyncChunkGen(const FVector& PlayerPosition);
	void StartAsyncChunkUpdate(const FVector& Position, int RenderDistance, const FIntVector& Direction);

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

	
	//Async Tasks
	FGraphEventRef PreviousTask = nullptr;
	FGraphEventArray TasksList;
	void GenerateChunkAsync(const FVector& PlayerPosition);
	void GenerateChunkAsyncComplete();
	void UpdateChunkAsync(const FVector& Position, int RenderDistance, const FIntVector& Direction);
	void UpdateChunkAsyncComplete();	

	//Mesh
	void GenerateMesh();
	void CreateQuad(FMask Mask, FIntVector AxisMask, int Width, int Height, FVector V1, FVector V2, FVector V3, FVector V4);
	void ApplyMesh();
	void ClearMeshData();

	//Utils
	int GetBlockIndex(int X, int Y, int Z) const;
	EBlock GetBlock(FIntVector Index, bool checkOutsideChunk);
	bool CompareMask(const FMask M1, const FMask M2) const;
	TArray<FIntVector> CalculatePerspectiveMask(FVector PlayerPosition);
	bool CompareNormalMask(FIntVector Normal);
	int GetTextureIndex(EBlock Block, FVector Normal) const;
	void PostStats(std::chrono::high_resolution_clock::time_point StartTime);
};
