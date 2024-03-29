// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "../Utils/Enums.h"
#include "../Utils/FastNoiseLite.h"
#include "../Utils/ChunkMeshData.h"

#include <chrono>

#include "Chunk.generated.h"
class UProceduralMeshComponent;

UCLASS()
class AChunk : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChunk();

	struct FMask
	{
		EBlock Block;
		int Normal;
	};

	UPROPERTY(EditAnywhere, Category = "Chunk")
	int ChunkSize = 32;

	int BlockSize = 50;
	int WorldScale = 50;
	FVector ChunkPosition;

	UPROPERTY(EditAnywhere, Category = "Chunk")
	int Lod = 1;

	TObjectPtr<UMaterialInterface> Material;
	UPROPERTY(EditAnywhere, Category = "Chunk")
	float Frequency = 0.014;

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	void ModifyVoxel(const FIntVector Position, const EBlock Block);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Setup();

	void StartAsyncChunkGen();
	
	virtual void ModifyVoxelData(const FIntVector Position, const EBlock Block);
	void GenerateBlocksFromNoise(FVector Position);

	TObjectPtr<UProceduralMeshComponent> Mesh;
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
	virtual void GenerateMesh();
	void CreateQuad(FMask Mask, FIntVector AxisMask, int Width, int Height, FIntVector V1, FIntVector V2, FIntVector V3, FIntVector V4);
	bool CompareMask(const FMask M1, const FMask M2) const;
	void ApplyMesh() const;
	void ClearMesh();

	int GetTextureIndex(EBlock Block, FVector Normal) const;

	std::chrono::high_resolution_clock::time_point StartTime;
	void PostStats();
	virtual void Tick(float DeltaTime) override;
};
