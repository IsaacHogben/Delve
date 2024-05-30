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

	// Default constructor
	FChunkMeshData() {}

	// Copy constructor
	FChunkMeshData(const FChunkMeshData& Other)
	{
		Vertices = Other.Vertices;
		Triangles = Other.Triangles;
		Normals = Other.Normals;
		Colors = Other.Colors;
		UV0 = Other.UV0;
	}

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

