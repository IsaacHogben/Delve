#pragma once

#include "CoreMinimal.h"

#include "ChunkMeshData.generated.h"

USTRUCT()
struct FChunkMeshData
{
	GENERATED_BODY();

public:
	UPROPERTY()
	TArray<FVector> Vertices;
	UPROPERTY()
	TArray<int> Triangles;
	UPROPERTY()
	TArray<FVector> Normals;
	UPROPERTY()
	TArray<FColor> Colors;
	UPROPERTY()
	TArray<FVector2D> UV0;

	void Clear();
};

inline void FChunkMeshData::Clear()
{
	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	Colors.Empty();
	UV0.Empty();
}

