// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGen/ChunkManager.h"
#include "WorldGen/ChunkRenderDistance.h"

// Sets default values
AChunkManager::AChunkManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

//AChunkManager::~AChunkManager()
//{
//	ChunkInstances.erase(std::remove(ChunkInstances.begin(), ChunkInstances.end(), this), ChunkInstances.end());
//}

void AChunkManager::UpdatePlayerChunkPosition(const FVector& Position)
{
	//BenchmarkTimer t;
	int i = 0;
	for (ChunkClass* instance : ChunkInstances) {
		instance->RenderDistanceUpdate(Position, RenderDistance);
		i++;
	}
	UE_LOG(LogTemp, Warning, TEXT("%d, chunks updated."), i);
	//t.LogTime();

}

// Called when the game starts or when spawned
void AChunkManager::BeginPlay()
{
	Super::BeginPlay();

	GenerateChunks();
}

void AChunkManager::GenerateChunks()
{
	ChunkRenderDistance crd(RenderDistance);
	TArray<ChunkRenderDistance::ChunkSpawnData> dataArray = crd.CalculateRenderSphere();
	for (ChunkRenderDistance::ChunkSpawnData& data : dataArray)
		SpawnChunk(data.Position, data.Lod);
}

void AChunkManager::SpawnChunk(FIntVector ChunkPos, int Lod)
{
	ChunkPos += FIntVector(0, 0, -1);//temp to not spawn camera in terrain
	ChunkClass *chunk = new ChunkClass();//TODO can move this assignments to the constructor
	chunk->ChunkManager = this;
	chunk->Lod = Lod;
	chunk->ChunkPosition = FVector(ChunkPos.X * ChunkSize * 50, ChunkPos.Y * ChunkSize * 50, ChunkPos.Z * ChunkSize * 50);
	chunk->BeginPlay();
	ChunkInstances.push_back(chunk);
}

UProceduralMeshComponent* AChunkManager::CreateMeshSection(FChunkMeshData MeshData, FVector Transform, int Vertexes, int Lod)
{
	// Create a new procedural mesh component
	UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(this);
	if (Mesh)
	{		
		// Attach the new mesh component to the root component or any other existing component of the actor
		Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		
		Mesh->bUseAsyncCooking = true;
		//Mesh->SetMobility(EComponentMobility::Static);		
		TObjectPtr<UMaterialInterface> Material;
		Mesh->SetMaterial(0, Material);

		UpdateMeshSection(Mesh, MeshData, Transform, Lod);

		// Optionally, you can also register the component with the scene so it can be rendered and updated
		Mesh->RegisterComponent();
	}
	else
	{
		// Handle failure to create the new mesh component
		UE_LOG(LogTemp, Warning, TEXT("Failed to create new mesh component"));
	}
	//UE_LOG(LogTemp, Warning, TEXT("MeshComplete"));
	return Mesh;
}

void AChunkManager::UpdateMeshSection(UProceduralMeshComponent* Mesh, FChunkMeshData MeshData, FVector Transform, int Lod)
{
	//UE_LOG(LogTemp, Warning, TEXT("updatedmesh"));
	//BenchmarkTimer timer;
	Mesh->ClearAllMeshSections();
	// Calculate Transform
	//UE_LOG(LogTemp, Warning, TEXT("pos > %f,%f,%f"), Transform.X, Transform.Y, Transform.Z);
	auto MeshTransform = FTransform(
		FRotator::ZeroRotator,
		FVector(Transform.X * WorldScale, Transform.Y * WorldScale, Transform.Z * WorldScale),
		FVector::OneVector
	);
	Mesh->SetRelativeTransform(MeshTransform);
	//Set Properties based on the meshes Lod
	if (Lod >= 2 && Lod != 0)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCastShadow(false);
	}
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
	//UE_LOG(LogTemp, Warning, TEXT("updatedmesh"));
	//timer.LogTime();
}

// Called every frame
void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

