#pragma once

#include "CoreMinimal.h"
#include "../WorldGen/ChunkClass.h"
#include "ChunkStructs.Generated.h"

class UChunkClass;

USTRUCT()
struct FChunkSpawnData
{
	GENERATED_BODY();

public:
	int Lod;
	FIntVector Position;
	UPROPERTY()
	TObjectPtr<UChunkClass> Chunk;

	bool operator<(const FChunkSpawnData& Other) const
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