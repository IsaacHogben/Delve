// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkRenderDistance.h"
#include "ChunkInclude.h"

#include "../Utils/ChunkMeshData.h"
#include "../Utils/VectorFunctionUtils.h"

#include <iostream>

#include "ChunkManager.generated.h"

class UChunkClass;
struct FChunkSpawnData;
struct FBlockUpdate;
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
	float MainFreqency = 0.015;

	UPROPERTY(EditAnywhere, Category = "Generation Settings")
	TObjectPtr<UMaterialInterface> Material;

	UProceduralMeshComponent* CreateMeshSection(FChunkMeshData* MeshData, FVector Transform, int Vertexes, int Lod);

	void UpdateMeshSection(UProceduralMeshComponent* Mesh, FChunkMeshData MeshData, FVector Transform, int Lod, int Vertices);
	
	UPROPERTY()
	FIntVector PreviousPlayerChunkPosition;

	void EnqueueMeshUpdate(UProceduralMeshComponent* Mesh, FChunkMeshData MeshData, FVector ChunkWorldPosition, int Lod, int VertexCount);

	void DistributeBulkChunkUpdates(TArray<FBlockUpdate> BlockUpdates);
	void UpdateChunkGenerationLayerStatus();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void GenerateChunks(FIntVector CenterPoint);
	void SpawnChunk(FChunkSpawnData dataArray, FIntVector CentralRenderChunkVector, int id);

private:
	int TotalChunks;
	int ChunksCompletedLayerOneGenration; //use array when more layers are nescessary.

	void UpdatePlayerChunkPositionAsync(const FVector& PlayerPosition);

	UPROPERTY()
	TArray<FChunkSpawnData> ChunkObjects;
	UPROPERTY()
	FIntVector LastUpdateDirection;

	FGraphEventArray UpdateTasksList;

	TQueue<FQueuedMeshUpdate> MeshUpdateQueue;
	//UProceduralMeshComponent* Mesh;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
