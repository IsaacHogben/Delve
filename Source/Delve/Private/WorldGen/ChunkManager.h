// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkRenderDistance.h"
#include "ChunkInclude.h"
#include "NoiseManager.h"
#include "ChunkLoader.h"

#include "../Utils/ChunkMeshData.h"
#include "../Utils/VectorFunctionUtils.h"

#include <iostream>

#include "ChunkManager.generated.h"

class UChunkClass;
class UNoiseManager;
class AProceduralTerrain;
struct FChunkData;
struct FBlockUpdate;
struct FCachedBlockUpdate;
struct FQueuedMeshUpdate;

UCLASS()
class AChunkManager : public AActor
{
	GENERATED_BODY()
	
public:	

	// Sets default values for this actor's properties
	AChunkManager();
	~AChunkManager();

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	void UpdatePlayerChunkPosition(const FVector& Position);

	int ChunkSize = 64;
	int WorldScale = 50;

	UPROPERTY(EditAnywhere, Category = "Quality")
	int RenderDistance = 1;

	UPROPERTY(EditAnywhere, Category = "Generation Settings")
	TObjectPtr<AProceduralTerrain> TerrainGenerator;

	UPROPERTY(EditAnywhere, Category = "Material Settings")
	TObjectPtr<UMaterialInterface> Material;

	UPROPERTY(EditAnywhere, Category = "Debug Settings")
	bool DrawNullBlocks = false;

	UProceduralMeshComponent* CreateMeshSection(FChunkMeshData* MeshData, FVector Transform, int Vertexes, int Lod);

	void UpdateMeshSection(UProceduralMeshComponent* Mesh, FChunkMeshData& MeshData, FVector Transform, int Lod, int Vertices);
	
	UPROPERTY()
	FIntVector PreviousPlayerChunkPosition;

	void EnqueueMeshUpdate(UProceduralMeshComponent* Mesh, FChunkMeshData& MeshData, FVector ChunkWorldPosition, int Lod, int VertexCount);
	void DistributeBulkChunkUpdates(TArray<FBlockUpdate> BlockUpdates);
	void UpdateChunkGenerationLayerStatus();
	void StartDecorationApplication(TSharedPtr<FChunkData> ChunkData);
	EBlock GetBlockFromChunk(const FIntVector& BlockIndex, const FIntVector& ChunkIndex);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void GenerateChunks(FIntVector CenterPoint);
	bool CheckChunkForNeighbours(TSharedPtr<FChunkData> ChunkData);
	void SpawnChunk(FChunkData dataArray, FIntVector CentralRenderChunkVector);

private:
	UPROPERTY()
	UNoiseManager* NoiseManager;

	int TotalChunks;
	int ChunkGenerationLayersExpected[3]; //use array when more layers are nescessary.

	void UpdatePlayerChunkPositionAsync(const FVector& PlayerPosition);
	void StartChunkGeneration();
	//UPROPERTY()
	// Using TMap to store chunk data
	TMap<FIntVector, TSharedPtr<FChunkData>> ActiveChunkMap;
	TMap<FIntVector, TSharedPtr<FChunkData>> InActiveChunkMap;
	TMap<FIntVector, TArray<FCachedBlockUpdate>> CachedChunkUpdateMap;

	UPROPERTY()
	FIntVector LastUpdateDirection;

	FGraphEventArray UpdateTasksList;
	bool GetSixPointers(TSharedPtr<FChunkData> Chunk);

	TQueue<FQueuedMeshUpdate> MeshUpdateQueue;
	//UProceduralMeshComponent* Mesh;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void CleanUpCachedData(TSharedPtr<FChunkData> ChunkData);
	FCriticalSection CriticalQueuedBlockUpdateAddSection;
	UExecutionTimer* UpdateProfileTimer;
	void ChunkLoadTest(); 
};
