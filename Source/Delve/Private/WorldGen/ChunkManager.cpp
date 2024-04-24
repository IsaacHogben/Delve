// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGen/ChunkManager.h"
//#include "WorldGen/ChunkRenderDistance.h"

// Sets default values
AChunkManager::AChunkManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PreviousPlayerChunkPosition = FIntVector();
	LastUpdateDirection = FIntVector();
}

AChunkManager::~AChunkManager()
{
	/*for (UChunkClass* instance : ChunkInstances) {
		delete instance;
	}
	ChunkInstances.clear();*/

}

void AChunkManager::UpdatePlayerChunkPosition(const FVector& PlayerPosition)
{
	FGraphEventRef UpdateTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this, PlayerPosition]() {
		UpdatePlayerChunkPositionAsync(PlayerPosition);
		}, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);
	
	UpdateTasksList.Add(UpdateTask);
}

void AChunkManager::UpdatePlayerChunkPositionAsync(const FVector& PlayerPosition)
{
	UE_LOG(LogTemp, Warning, TEXT("Updating..."));
	bool UpdateDirection = false;
	FIntVector NewPlayerChunkPosition = VectorFunctionUtils::FVectorToFIntVector(PlayerPosition);

	FIntVector Direction = NewPlayerChunkPosition - PreviousPlayerChunkPosition;
	if (Direction != FIntVector(0, 0, 0))
	{		
		TArray<FIntVector> AvailablePositions;
		TArray<FChunkSpawnData*> AvailableChunks;
		
		for (int i = 0; i < ChunkObjects.Num(); i++)
		{
			float OutChunkDistance = FIntVectorDistance(NewPlayerChunkPosition, ChunkObjects[i].Position);
			float InChunkDistance = FIntVectorDistance(PreviousPlayerChunkPosition, ChunkObjects[i].Position + Direction);

			if (InChunkDistance > RenderDistance)// Get a list of new positions to fill with available chunks
			{
				AvailablePositions.Add(ChunkObjects[i].Position + Direction);// Could be a predifined list
			}

			if (OutChunkDistance > RenderDistance)// If out of render distance make class available and remove from list
			{
				AvailableChunks.Add(&ChunkObjects[i]);
			}
			else //if not update render distance
				ChunkObjects[i].Chunk->StartAsyncChunkLodUpdate(RenderDistance, OutChunkDistance, PlayerPosition);
		}

		int i = 0;
		for (auto& instance : AvailableChunks)// Match available positions and chunks and update.
		{
			instance->Position = AvailablePositions[i++];
			instance->Chunk->StartAsyncChunkPositionUpdate(PlayerPosition, instance->Position);
		}
	}
	PreviousPlayerChunkPosition = NewPlayerChunkPosition;
	UE_LOG(LogTemp, Warning, TEXT("Update finished"));
}

// Called when the game starts or when spawned
void AChunkManager::BeginPlay()
{
	Super::BeginPlay();

	GenerateChunks(PreviousPlayerChunkPosition);
}

void AChunkManager::GenerateChunks(FIntVector CentralRenderChunkVector)
{
	//Create the Mesh component
	//Mesh = NewObject<UProceduralMeshComponent>(this);

	ChunkRenderDistance crd(RenderDistance);
	TArray<FChunkSpawnData> dataArray = crd.CalculateRenderSphere();
	int i = 0;
	for (FChunkSpawnData& data : dataArray)
	{
		SpawnChunk(data, CentralRenderChunkVector);
		i++;
	}
	UE_LOG(LogTemp, Warning, TEXT("%d Chunks Spawned."), i);

}

void AChunkManager::SpawnChunk(FChunkSpawnData data, FIntVector CentralRenderChunkVector)
{
	FIntVector position = (data.Position + CentralRenderChunkVector) * ChunkSize;
	UChunkClass* chunk = NewObject<UChunkClass>();//TODO can move this assignments to the constructor
	chunk->ChunkManager = this;
	chunk->Lod = data.Lod;
	chunk->ChunkWorldPosition = FVector(position);
	chunk->CentralRenderChunkVector = CentralRenderChunkVector;
	chunk->BeginPlay();

	FChunkSpawnData ChunkData = FChunkSpawnData();
	ChunkData.Lod = data.Lod;
	ChunkData.Position = data.Position;
	ChunkData.Chunk = chunk;
	ChunkObjects.Add(ChunkData);
}

UProceduralMeshComponent* AChunkManager::CreateMeshSection(FChunkMeshData* MeshData, FVector Transform, int Vertexes, int Lod)
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
		
		UpdateMeshSection(Mesh, MeshData, Transform, Lod, Vertexes);

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

void AChunkManager::UpdateMeshSection(UProceduralMeshComponent* Mesh, FChunkMeshData* MeshData, FVector Transform, int Lod, int Vertexes)
{
	//RuntimeMeshComponent
	Mesh->ClearAllMeshSections();
	if (!Vertexes)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Skipped chunk with %d Vertexes"), Vertexes);
		return;
	}	

	// Calculate Transform
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
	else //Properties for LOD 1 and 2
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCastShadow(false);
	}
	Mesh->CreateMeshSection(
		0,
		MeshData->Vertices,
		MeshData->Triangles,
		MeshData->Normals,
		MeshData->UV0,
		MeshData->Colors,
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

