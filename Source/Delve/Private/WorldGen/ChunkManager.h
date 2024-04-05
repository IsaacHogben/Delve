// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkInclude.h"
#include "ChunkClass.h"
#include "../Utils/ChunkMeshData.h"

#include <iostream>
#include <vector>

#include "ChunkManager.generated.h"

class ChunkClass;

UCLASS()
class AChunkManager : public AActor
{
	GENERATED_BODY()
	
public:	

	// Sets default values for this actor's properties
	AChunkManager();

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	void UpdatePlayerChunkPosition(const FVector& Position);

	int ChunkSize = 32;
	int WorldScale = 50;
	int RenderDistance = 24;
	UProceduralMeshComponent* CreateMeshSection(FChunkMeshData MeshData, FVector Transform, int Vertexes, int Lod);

	void UpdateMeshSection(UProceduralMeshComponent* Mesh, FChunkMeshData MeshData, FVector Transform, int Lod);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void GenerateChunks();
	void SpawnChunk(FIntVector position, int Lod);

private:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	std::vector<ChunkClass*> ChunkInstances;
};
