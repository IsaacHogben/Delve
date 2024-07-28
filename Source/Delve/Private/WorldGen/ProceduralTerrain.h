// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkInclude.h"
#include "../Utils/FastNoiseLite.h"
#include <stack>
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

#include "GenClasses/Truce/BaseRegion.h"
#include "GenClasses/Truce/CliffRegion.h"
#include "GenClasses/Truce/TopRegion.h"
#include "GenClasses/ProceduralParts.h"
#include "GenClasses/GenUtils.h"

#include "ProceduralTerrain.generated.h"

class UNoiseManager;
struct FCachedBlockUpdate;
struct FFastNoise;
struct FTreeSystem;
/**
 * 
 */
struct LSystem {
	FString Axiom;
	TMap<TCHAR, FString> Rules;
	float Angle;
	float AngleDeviation;
	float TrunkDeviation;
	int BaseBranchLength;
	int BaseTrunkRadius;
};

UCLASS()
class AProceduralTerrain : public AActor
{
	GENERATED_BODY()

public:
	AProceduralTerrain();
	~AProceduralTerrain();
	void Initialize(int _WorldRadius);
	UPROPERTY()
	UNoiseManager* N;

	UPROPERTY(EditAnywhere, Category = "Generation Settings")
	int MaxBlockSpawnHeight = 85;
	UPROPERTY(EditAnywhere, Category = "Generation Settings")
	TArray<FFastNoise> GenerationNoiseArray;

	// The reference to the curve asset
	UPROPERTY(EditAnywhere, Category = "Curves")
	UCurveFloat* ZDensityCurve;
	UPROPERTY(EditAnywhere, Category = "Curves")
	UCurveFloat* WorldEdgeDensityCurve;
	UPROPERTY(EditAnywhere, Category = "Curves")
	UCurveFloat* WorldEdgeDropoffCurve;

	UPROPERTY(EditAnywhere, Category = "DataTables")
	TObjectPtr<UDataTable> TreeDataTable;

	UPROPERTY(EditAnywhere, Category = "Instanced")
	UHierarchicalInstancedStaticMeshComponent* HISMC;
	UPROPERTY(EditAnywhere, Category = "Instanced")
	UStaticMesh* IMGrass;
	UPROPERTY(EditAnywhere, Category = "Instanced")
	UMaterialInterface* GrassMat;

	FIntVector2 WorldCenter = FIntVector2(32, 32);
	int ChunkSize = 64;
	int GetBlockIndex(int X, int Y, int Z);
	
	TArray<FCachedBlockUpdate> GetGeneratedChunk(FVector ChunkPosition, FIntVector ChunkVectorPosition, TArray<EBlock>& BlockArray, bool& isChunkEmpty);

	// Uses the generated terrain array to add decorations.
	TArray<FCachedBlockUpdate> AddDecorationsWithContext(TArray<EBlock>& BlockArray, TArray<FVector>& InstancedMeshPositions, UChunkClass* Chunk, FVector& ChunkPosition);
	void ApplyInstancedFoliage(TArray<FVector>& Positions);

protected:


private:
	EBlock GetTerrainLevelOne(float x, float y, float z, EBlock AboveBlock);
	//float GetNoiseLevelOne(float x, float y, float z);
	TArray<FTreeSystem*> TreeDataArray;
	//bool IsInLocalRegion(FastNoiseLite* Region, float RegionSize, float& x, float& y, float& z);
	EBlock GetBlockFromRegion(ULocalRegion* LocalRegion, ESoilLayer SoilLayer);

	int WorldChunkRadius;

	bool IsSurfaceBlock(float AboveValue, float Density);
	bool IsAir(float Value, float Density);

	void AddReferencelessDecorations(TArray<EBlock>& BlockArray, FastNoiseLite* Noise, TArray<FCachedBlockUpdate>& BlockUpdates);

	void MakeTestShape(TArray<FCachedBlockUpdate>& BlockUpdates, int x, int y, int z);

	void MakeTestVine(TArray<FCachedBlockUpdate>& BlockUpdates, int x, int y, int z);
	float GetDomeHeight(const float& Radius, const float& Distance);
	void AddCylinder(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int height, int centerX, int centerY, int baseZ, EBlock blockType);
	void AddSphere(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int centerX, int centerY, int centerZ, EBlock blockType);
	void AddCanopy(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int centerX, int centerY, int centerZ, EBlock blockType, uint8 density);
	void GenerateTestTreeAtLocation(TArray<FCachedBlockUpdate>& BlockUpdates, UChunkClass* Chunk, int x, int y, int z);
	void GenerateTree(TArray<FCachedBlockUpdate>& BlockUpdates, UChunkClass* Chunk, int x, int y, int z, const FTreeSystem& system);

	//Local Regions
	UPROPERTY()
	class UBaseRegion* BaseRegion;
	UPROPERTY()
	class UCliffRegion* CliffRegion;
	UPROPERTY()
	class UTopRegion* TopRegion;

};

int QuantizeCoordinate(int value, int quantizationStep);

float GetQuantizedNoise(int x, int y, int z, FastNoiseLite* Noise);
