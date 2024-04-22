// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkRenderDistance.h"
#include "ChunkInclude.h"
#include "../Utils/ChunkMeshData.h"
//#include "../Utils/ChunkSpawnData.h"
#include "../Utils/VectorFunctionUtils.h"

#include <iostream>

#include "ChunkManager.generated.h"

class UChunkClass;
struct FChunkSpawnData;

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

	int ChunkSize = 32;
	int WorldScale = 50;
	UPROPERTY(EditAnywhere, Category = "ChunkManager")
	int RenderDistance = 1;
	UProceduralMeshComponent* CreateMeshSection(FChunkMeshData* MeshData, FVector Transform, int Vertexes, int Lod);

	void UpdateMeshSection(UProceduralMeshComponent* Mesh, FChunkMeshData* MeshData, FVector Transform, int Lod);
	
	UPROPERTY()
	FIntVector PlayerChunkPosition;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void GenerateChunks(FIntVector CenterPoint);
	void SpawnChunk(FChunkSpawnData dataArray, FIntVector CentralRenderChunkVector);

private:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//FChunkSpawnData* 
	UPROPERTY()
	TArray<FChunkSpawnData> ChunkObjects;
	UPROPERTY()
	FIntVector LastUpdateDirection;
};
