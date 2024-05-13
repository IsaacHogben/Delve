// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGen/ChunkManager.h"
//#include "WorldGen/ChunkRenderDistance.h"

// Sets default values
AChunkManager::AChunkManager()
{
	PreviousPlayerChunkPosition = FIntVector();
	LastUpdateDirection = FIntVector();

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.TickInterval = 0.1f;
	// 
	//UpdateMeshDelegate.BindUFunction(this, FName("UpdateMeshSection"));
	

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
	//UpdatePlayerChunkPositionAsync(PlayerPosition);

	//Octree Test
	//FOctree<EBlock> Octree(FRegion(FPoint(0,0,0), FPoint(ChunkSize, ChunkSize, ChunkSize)), EBlock::Null);

	//FPoint p1 = { 64,64,64 };
	//FPoint p2 = { 19,26,37 };

	//Octree.Insert(p1, EBlock::Dirt);

	//EBlock QueryResult = Octree.QueryPoint(p1);
	//EBlock QueryResult2 = Octree.QueryPoint(p1);
	//EBlock QueryResult3 = Octree.QueryPoint(p2);

	//// Print the query result
	//UE_LOG(LogTemp, Warning, TEXT("Block Value: %d"), QueryResult);
	//UE_LOG(LogTemp, Warning, TEXT("Block Value: %d"), QueryResult2);
	//UE_LOG(LogTemp, Warning, TEXT("Block Value: %d"), QueryResult3);

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
		TArray<FChunkData*> AvailableChunks;
		
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
		UE_LOG(LogTemp, Warning, TEXT("Positions: %d/n Chunks: %d"), AvailablePositions.Num(), AvailableChunks.Num());

		int i = 0;
		for (auto& chunk : AvailableChunks)// Match available positions and chunks and update.
		{
			chunk->Position = AvailablePositions[i++];
			chunk->Chunk->StartAsyncChunkPositionUpdate(PlayerPosition, chunk->Position);
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
	TArray<FChunkData> dataArray = crd.CalculateRenderSphere();
	int i = 0;
	for (FChunkData& data : dataArray)
	{
		SpawnChunk(data, CentralRenderChunkVector, i);
		i++;
	}
	ChunkObjects.Sort();
	TotalChunks = i;
	UE_LOG(LogTemp, Warning, TEXT("%d Chunks Spawned."), i);

}

void AChunkManager::SpawnChunk(FChunkData data, FIntVector CentralRenderChunkVector, int i)
{
	FIntVector position = (data.Position + CentralRenderChunkVector) * ChunkSize;
	UChunkClass* chunk = NewObject<UChunkClass>();//TODO can move this assignments to the constructor
	chunk->ChunkManager = this;
	chunk->Frequency = MainFreqency;
	chunk->Lod = data.Lod;
	chunk->ChunkWorldPosition = FVector(position);
	chunk->CentralRenderChunkVector = CentralRenderChunkVector;
	chunk->id = i;
	chunk->BeginPlay();

	FChunkData* ChunkData =  new FChunkData();
	ChunkData->Lod = data.Lod;
	ChunkData->Position = data.Position;
	ChunkData->Chunk = chunk;
	ChunkData->Blocks = new FOctree<EBlock>(FRegion(FPoint(0,0,0), FPoint(ChunkSize, ChunkSize, ChunkSize)), EBlock::Null);
	chunk->ChunkData = ChunkData;

	ChunkObjects.Add(*ChunkData);
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

		Mesh->SetMaterial(0, Material);
		
		EnqueueMeshUpdate(Mesh, *MeshData, Transform, Lod, Vertexes);

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

void AChunkManager::UpdateMeshSection(UProceduralMeshComponent* Mesh, FChunkMeshData MeshData, FVector Transform, int Lod, int Vertexes)
{
	if (!Vertexes)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Skipped chunk with %d Vertexes"), Vertexes);
		Mesh->ClearAllMeshSections();
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
		MeshData.Vertices,
		MeshData.Triangles,
		MeshData.Normals,
		MeshData.UV0,
		MeshData.Colors,
		TArray<FProcMeshTangent>(),
		true
	);

	MeshData.Clear();
	//UE_LOG(LogTemp, Warning, TEXT("Updated Mesh with %d Vertexes"), Vertexes);

}

// Enqueue a mesh update function
void AChunkManager::EnqueueMeshUpdate(UProceduralMeshComponent* Mesh, FChunkMeshData MeshData, FVector ChunkWorldPosition, int Lod, int VertexCount)
{
	FQueuedMeshUpdate Update = FQueuedMeshUpdate();
	Update.Mesh = Mesh;
	Update.MeshData = MeshData;
	Update.Transform = ChunkWorldPosition;
	Update.Lod = Lod;
	Update.Vertexes = VertexCount;

	MeshUpdateQueue.Enqueue(Update);
}

void AChunkManager::DistributeBulkChunkUpdates(TArray<FBlockUpdate> BlockUpdates)
{
	int n = 0;
	for (int j = 0; j < ChunkObjects.Num(); j++)
	{
		for (int i = 0; i < BlockUpdates.Num(); i++)
		{
			if (ChunkObjects[j].Position == BlockUpdates[i].TargetChunk)
			{
				ChunkObjects[j].QueuedBlockUpdates.Add(FBlockUpdate(BlockUpdates[i].TargetChunk, BlockUpdates[i].DispatchChunk, BlockUpdates[i].Position, BlockUpdates[i].Block));
				UE_LOG(LogTemp, Warning, TEXT("Enqueued %d.%d.%d"), BlockUpdates[i].Position.X, BlockUpdates[i].Position.Y, BlockUpdates[i].Position.Z);
				n++;
				//UE_LOG(LogTemp, Warning, TEXT("Enqueued %d"), ChunkObjects[j].QueuedBlockUpdates.Num());
			}
		}
	}
}

void AChunkManager::UpdateChunkGenerationLayerStatus()
{
	if (++ChunksCompletedLayerOneGenration == TotalChunks)
	{
		UE_LOG(LogTemp, Warning, TEXT("Layer 1 complete"));
		for (int j = 0; j < ChunkObjects.Num(); j++)
		{
			//UE_LOG(LogTemp, Warning, TEXT("que %d"), ChunkObjects[j].QueuedBlockUpdates.Num());
			ChunkObjects[j].Chunk->ModifyVoxels(ChunkObjects[j].QueuedBlockUpdates, true);
			ChunkObjects[j].QueuedBlockUpdates.Empty();
		}
	}	
	//UE_LOG(LogTemp, Warning, TEXT("ChunksCompletedLayerOneGenration: %d"), ChunksCompletedLayerOneGenration);
	//UE_LOG(LogTemp, Warning, TEXT("Total Chunks: %d"), TotalChunks);
}

// Called every frame
void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Define the maximum number of updates to process per tick
	const int32 MaxUpdatesPerTick = 1; // Adjust as needed
	// Counter to track the number of updates processed
	int32 UpdatesProcessedThisTick = 0;
	while (!MeshUpdateQueue.IsEmpty() && UpdatesProcessedThisTick < MaxUpdatesPerTick)
	{	
		FQueuedMeshUpdate Update;
		MeshUpdateQueue.Dequeue(Update);

		UpdateMeshSection(Update.Mesh, Update.MeshData, Update.Transform, Update.Lod, Update.Vertexes);
		
		UpdatesProcessedThisTick++;
	}
}

