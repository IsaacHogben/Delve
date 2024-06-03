// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkInclude.h"
#include "ChunkRenderDistance.h"
//#include "ProceduralTerrain.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "NoiseManager.h"

#include "Misc/DateTime.h"
#include "../Utils/Enums.h"
#include "../Utils/FastNoiseLite.h"
#include "../Utils/ChunkMeshData.h"
#include "../Utils/VectorFunctionUtils.h"
#include <chrono>

#include "ChunkClass.generated.h"

class UProceduralMeshComponent;
class AChunkManager; // Forward declaration of AChunkManager
class AProceduralTerrain;

struct FBlockUpdate;
struct FCachedBlockUpdate;
struct FChunkData;

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
	
	void BeginGeneration();

	void Setup();

	UPROPERTY()
	AChunkManager* ChunkManager;
	UPROPERTY()
	UProceduralMeshComponent* Mesh;
	UPROPERTY()
	AProceduralTerrain* TerrainGenerator;
	TSharedPtr<FChunkData> ChunkData;

	//Variables
	UPROPERTY(EditAnywhere, Category = "Chunk")
	int ChunkSize = 64;
	int BlockSize = 50;
	int WorldScale = 50;
	
	UPROPERTY()// Represents the chunks position in the world. Used for Noise sample pos.
	FVector ChunkWorldPosition;
	
	//UPROPERTY()// Represents the chunks position vector as whole number;
	//FIntVector ChunkVectorPosition;

	UPROPERTY(EditAnywhere, Category = "Chunk")
	int Lod = 1;
	TObjectPtr<UMaterialInterface> Material;

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	FIntVector ModifyVoxel(FIntVector& Position, const EBlock& Block, bool RegenerateMesh);
	void ModifyVoxels(TArray<FCachedBlockUpdate>& BlockUpdates, bool RegenerateMesh);

	void ApplyMesh();
	void ClearMesh();
	EBlock GetBlock(FIntVector Index, bool checkOutsideChunk);

	void StartAsyncChunkLodUpdate(int RenderDistance, const float Distance, const FVector PlayerPosition);
	void StartAsyncChunkPositionUpdate();

protected:
	
	void StartAsyncChunkGen(const FVector& PlayerPosition);

	void ModifyVoxelData(const FIntVector Position, const EBlock Block);
	void GenerateProceduralTerrain();
	FastNoiseLite* Noise;

	UPROPERTY()
	int VertexCount = 0;

private:

	UPROPERTY()
	TArray<FIntVector> PerspectiveMask;
	FGraphEventArray TasksList;
	FChunkMeshData* MeshData;
	
	bool IsChunkEmpty = true;

	void UpdateChunkLodAsync(int RenderDistance, const float Distance, const FVector PlayerPosition);
	void GetLod(int RenderDistance, const float& Distance, bool& ContinueToUpdate);
	void UpdatePerspectiveMask(const FVector& PlayerPosition, bool& ContinueToUpdate);
	void UpdateChunkAsyncComplete();


	//Mesh
	void AGenerateMesh();
	void CreateQuad(FMask Mask, FIntVector AxisMask, int Width, int Height, FVector V1, FVector V2, FVector V3, FVector V4);
	void ClearMeshData();

	//Utils
	FIntVector GetBlockChunkAndIndex(FIntVector& Index);
	bool CompareMask(const FMask M1, const FMask M2) const;
	TArray<FIntVector> CalculatePerspectiveMask(FVector PlayerPosition);
	bool CompareNormalMask(FIntVector Normal);
	int GetTextureIndex(EBlock Block, FVector Normal) const;

	void TaskGraphDebugLog();

};
