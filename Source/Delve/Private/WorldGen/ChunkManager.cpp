// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGen/ChunkManager.h"

// Sets default values
AChunkManager::AChunkManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AChunkManager::BeginPlay()
{
	Super::BeginPlay();

	GenerateChunks();
}

void AChunkManager::GenerateChunks()
{
	int s = 16;
	int Lod = 1;
	TArray<int32> LodArray = {1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 8, 16, 32};
	//const auto Chunk = AChunk::StaticClass();
	FIntVector ChunkPos = FIntVector(0, 0, 0);
	for (int z = 0; z < s; z++)
	{
		for (int x = 0; x < s; x++)
		{
			Lod = 0;
			for (int y = 0; y < s; y++)
			{
				ChunkPos = FIntVector(x, y, -z);
				SpawnChunk(ChunkPos, LodArray[Lod]);
				Lod++;
			}
		}
	}
}

void AChunkManager::SpawnChunk(FIntVector ChunkPos, int Lod)
{
	ChunkClass *chunk = new ChunkClass();
	chunk->ChunkManager = this;
	chunk->Lod = Lod;
	chunk->ChunkPosition = FVector(ChunkPos.X * ChunkSize * 50, ChunkPos.Y * ChunkSize * 50, ChunkPos.Z * ChunkSize * 50);
	chunk->BeginPlay();
}

void AChunkManager::CreateMeshSection(FChunkMeshData MeshData, FVector Transform, int Vertexes)
{
	// Create a new procedural mesh component
	UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(this);
	if (Mesh)
	{
		// Calculate Transform
		auto MeshTransform = FTransform(
			FRotator::ZeroRotator,
			FVector(Transform.X * WorldScale, Transform.Y * WorldScale, Transform.Z * WorldScale),
			FVector::OneVector
		);
		// Attach the new mesh component to the root component or any other existing component of the actor
		Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		Mesh->SetRelativeTransform(MeshTransform);

		// Set any properties or parameters for the new mesh component
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// Set other properties as needed...
		Mesh->SetCastShadow(true);
		TObjectPtr<UMaterialInterface> Material;
		Mesh->SetMaterial(0, Material);
		Mesh->CreateMeshSection(
			0,
			MeshData.Vertices,
			MeshData.Triangles,
			MeshData.Normals,
			MeshData.UV0,
			MeshData.Colors,
			TArray<FProcMeshTangent>(),
			true
		);
		// Optionally, add the new mesh component to an array or container for later reference
		//MeshComponents.Add(NewMeshComponent);

		// Optionally, you can also register the component with the scene so it can be rendered and updated
		Mesh->RegisterComponent();
	}
	else
	{
		// Handle failure to create the new mesh component
		UE_LOG(LogTemp, Warning, TEXT("Failed to create new mesh component"));
	}
}

// Called every frame
void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

