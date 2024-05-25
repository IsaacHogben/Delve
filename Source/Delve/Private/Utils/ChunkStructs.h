#pragma once

#include "CoreMinimal.h"
#include "../WorldGen/ChunkClass.h"
#include "../WorldGen/Octree.h"
#include "ChunkStructs.Generated.h"

class UChunkClass;

USTRUCT()
struct FBlockUpdate
{
	GENERATED_BODY();

public:
	FIntVector TargetChunk;
	FIntVector Position;
	EBlock Block;

	// Constructor with all parameters
	FBlockUpdate(const FIntVector& InTargetChunk = FIntVector::ZeroValue,
		const FIntVector& InPosition = FIntVector::ZeroValue,
		EBlock InBlock = EBlock::Null)
		: TargetChunk(InTargetChunk),
		Position(InPosition),
		Block(InBlock)
	{
	}
};

USTRUCT()
struct FCachedBlockUpdate
{
	GENERATED_BODY();

public:
	FIntVector Position;
	EBlock Block;

	// Constructor with all parameters
	FCachedBlockUpdate(const FIntVector& InPosition = FIntVector::ZeroValue,
		EBlock InBlock = EBlock::Null)
		: Position(InPosition),
		Block(InBlock)
	{
	}
};

USTRUCT()
struct FChunkData
{
	GENERATED_BODY();

public:
	bool IsActive;
	int Lod;
	UPROPERTY()
	FIntVector Position;
	UPROPERTY()
	TObjectPtr<UChunkClass> Chunk;
	UPROPERTY()
	TArray<FCachedBlockUpdate> QueuedBlockUpdates;
	UPROPERTY()
	TArray<EBlock> Blocks;
	TArray<TSharedPtr<FChunkData>> NeighbourChunks;
	bool HasDistributedDecorations = false;
	ECompletedGenerationLayer GenerationLayer = ECompletedGenerationLayer::Empty;

	// Default constructor
	FChunkData()
		: IsActive(true)
		, Lod(1)
		, Position(FIntVector::ZeroValue)
		, Chunk(nullptr)
		, HasDistributedDecorations(false)
		, GenerationLayer(ECompletedGenerationLayer::Empty)
	{
	}
	~FChunkData()
	{
		UE_LOG(LogTemp, Error, TEXT("FChunkData Deconstructor Called!"));
	}

	bool operator<(const FChunkData& Other) const
	{
		// Compare LOD first
		if (Lod < Other.Lod)
		{
			return true;
		}
		// If LODs are equal, compare positions?
		// LOD is less than Other.Lod
		return false;
	}

	// Define hash function for FVector if necessary
	FORCEINLINE uint32 GetTypeHash(const FIntVector& Vector)
	{
		// Simple hash combining example for FVector
		return HashCombine(HashCombine(::GetTypeHash(Vector.X), ::GetTypeHash(Vector.Y)), ::GetTypeHash(Vector.Z));
	}
};

USTRUCT()
struct FQueuedMeshUpdate
{
	GENERATED_BODY();

public:
	UProceduralMeshComponent* Mesh;
	FChunkMeshData MeshData;
	FVector Transform;
	int Lod;
	int Vertexes;
};

