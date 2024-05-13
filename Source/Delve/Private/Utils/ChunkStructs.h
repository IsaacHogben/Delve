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
	FIntVector DispatchChunk;
	FIntVector Position;
	EBlock Block;

	// Constructor with all parameters
	FBlockUpdate(const FIntVector& InTargetChunk = FIntVector::ZeroValue,
		const FIntVector& InDispatchChunk = FIntVector::ZeroValue,
		const FIntVector& InPosition = FIntVector::ZeroValue,
		EBlock InBlock = EBlock::Null)
		: TargetChunk(InTargetChunk),
		DispatchChunk(InDispatchChunk),
		Position(InPosition),
		Block(InBlock)
	{
	}
};

USTRUCT()
struct FChunkData
{
	GENERATED_BODY();

public:
	int Lod;
	FIntVector Position;
	UPROPERTY()
	TObjectPtr<UChunkClass> Chunk;
	TArray<FBlockUpdate> QueuedBlockUpdates;
	FOctree<EBlock>* Blocks;
	int CompletedGenerationLayer = 0;

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
