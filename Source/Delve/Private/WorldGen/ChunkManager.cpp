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
	UE_LOG(LogTemp, Error, TEXT("ChunkManager Deconstrusctor Called!"));
}

void AChunkManager::UpdatePlayerChunkPosition(const FVector& PlayerPosition)
{
	UpdatePlayerChunkPositionAsync(PlayerPosition);
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
		TArray< TSharedPtr<FChunkData>> AvailableChunks;

		for (auto& Elem : ChunkMap)
		{
			TSharedPtr<FChunkData> ChunkData = Elem.Value;
			float OutChunkDistance = FIntVectorDistance(NewPlayerChunkPosition, ChunkData->Position);
			float InChunkDistance = FIntVectorDistance(PreviousPlayerChunkPosition, ChunkData->Position + Direction);

			if (InChunkDistance > RenderDistance)
			{
				AvailablePositions.Add(ChunkData->Position + Direction);
			}

			if (OutChunkDistance > RenderDistance)
			{
				AvailableChunks.Add(ChunkData);
				ChunkGenerationLayersExpected[int(EGenerationLayer::TerrainLayer)]++;
			}
			else// Update Lod
			{
				//Broken maybe but only a bit
				// still not worth using due to CreateMeshSection bottleneck
				//ChunkData->Chunk->StartAsyncChunkLodUpdate(RenderDistance, OutChunkDistance, PlayerPosition);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Positions: %d\nChunks: %d"), AvailablePositions.Num(), AvailableChunks.Num());

		int i = 0;
		for (auto& ChunkData : AvailableChunks)
		{
			if (i < AvailablePositions.Num())
			{
				FIntVector OldPosition = ChunkData->Position;
				FIntVector NewPosition = AvailablePositions[i++];

				ChunkMap.Remove(OldPosition);
				ChunkData->Position = NewPosition;
				ChunkMap.Add(NewPosition, ChunkData);
				//Reset Values
				ChunkData->Blocks.Empty();
				ChunkData->QueuedBlockUpdates.Empty();
				ChunkData->GenerationLayer = EGenerationLayer::TerrainLayer;
				ChunkData->NeighbourChunks.Empty();
				ChunkData->HasSixNeighbours = false;
			}
		}
		CheckAllChunksForNeighbours();
		for (auto& ChunkData : AvailableChunks)
		{
			ChunkData->Chunk->StartAsyncChunkPositionUpdate(PlayerPosition, ChunkData->Position);

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
	// Initialize render distance manager
	ChunkRenderDistance crd(RenderDistance);
	// Calculate render sphere
	TArray<FChunkData> ChunksToSpawn = crd.CalculateRenderSphere();

	// Spawn all chunks to be used for the rest of the game session
	for (FChunkData& chunk : ChunksToSpawn)
	{
		SpawnChunk(chunk, CentralRenderChunkVector);
		//chunk.Chunk->BeginGeneration();
		TotalChunks++;
	}
	ChunksToSpawn.Empty();
	ChunkGenerationLayersExpected[int(EGenerationLayer::TerrainLayer)] = TotalChunks;
	UE_LOG(LogTemp, Warning, TEXT("%d Chunks Spawned."), TotalChunks);

	CheckAllChunksForNeighbours();
}

// Must be done after chunks are spawned, neighbour information used for later generation stages
void AChunkManager::CheckAllChunksForNeighbours()
{
	for (auto& Elem : ChunkMap)
	{
		// Elem is of type TPair<FIntVector, FChunkData*>
		TSharedPtr <FChunkData> ChunkData = Elem.Value;
		if (!ChunkData->HasSixNeighbours && GetSixPointers(ChunkData))
		{
			ChunkGenerationLayersExpected[int(EGenerationLayer::InterChunkLayer)]++;
		}
	}
}

void AChunkManager::SpawnChunk(FChunkData data, FIntVector CentralRenderChunkVector)
{
	FIntVector position = (data.Position + CentralRenderChunkVector) * ChunkSize;
	UChunkClass* chunk = NewObject<UChunkClass>();//TODO can move this assignments to the constructor
	chunk->AddToRoot();
	chunk->ChunkManager = this;
	chunk->Frequency = MainFreqency;
	chunk->Lod = data.Lod;
	chunk->ChunkWorldPosition = FVector(position);

	TSharedPtr<FChunkData> ChunkData = MakeShared<FChunkData>();
	ChunkData->Lod = data.Lod;
	ChunkData->Position = data.Position;
	ChunkData->Chunk = chunk;
	chunk->ChunkData = ChunkData;

	ChunkMap.Add(ChunkData->Position, ChunkData);

	chunk->BeginGeneration();
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
	//UE_LOG(LogTemp, Warning, TEXT("mesh update queued"));
}

void AChunkManager::DistributeBulkChunkUpdates(TArray<FBlockUpdate> BlockUpdates)
{
	int n = 0;

	for (int i = 0; i < BlockUpdates.Num(); i++)
	{
		// Try to find the chunk in ChunkMap using the TargetChunk position from BlockUpdates
		TSharedPtr<FChunkData>* FoundChunkData = ChunkMap.Find(BlockUpdates[i].TargetChunk);
		if (FoundChunkData != nullptr)
		{
			// If found, add the block update to the chunk's QueuedBlockUpdates
			(*FoundChunkData)->QueuedBlockUpdates.Add(FBlockUpdate(BlockUpdates[i].TargetChunk, BlockUpdates[i].DispatchChunk, BlockUpdates[i].Position, BlockUpdates[i].Block));
			n++;
		}
		else
		{
			//what do about these ones
		}
	}
}

// Chunks send their current generation status here when it is complete to sync layers. All GenerationLayer changes go through here for continuity
void AChunkManager::UpdateChunkGenerationLayerStatus(EGenerationLayer GenerationLayer)
{
	//UE_LOG(LogTemp, Warning, TEXT("Layer %d at %d"), int(GenerationLayer), ChunkGenerationLayersExpected[int(GenerationLayer)]);
	//UE_LOG(LogTemp, Warning, TEXT("Called by chunk: %d.%d.%d"), int(GenerationLayer), ChunkGenerationLayersExpected[int(GenerationLayer)]);
	if (--ChunkGenerationLayersExpected[int(GenerationLayer)] == 0)
	{
		//start thread here
		UE_LOG(LogTemp, Warning, TEXT("Layer %d complete"), GenerationLayer);
		for (auto& Elem : ChunkMap)
		{
			
			// Elem is of type TPair<FIntVector, FChunkData*>
			TSharedPtr<FChunkData> ChunkData = Elem.Value;
			//UE_LOG(LogTemp, Warning, TEXT("que %d"), ChunkObjects[j].QueuedBlockUpdates.Num());
			//UE_LOG(LogTemp, Warning, TEXT("num negihbs in Up %d %d"), ChunkData->NeighbourChunks.Num(), ChunkData->HasSixNeighbours);

			switch (GenerationLayer)
			{
				case EGenerationLayer::TerrainLayer:
				{
				// In this step the chunk has generated the terrain and queued all decoration changes in other chunks
				// We sync this so that all the decorations are queued before we start applying any.
				// Only chunks with six Neigbours are eligible for this stage as they have received all possible decorations
					
					if (ChunkData->HasSixNeighbours && ChunkData->GenerationLayer == EGenerationLayer::TerrainLayer)
					{
						ChunkData->GenerationLayer = EGenerationLayer::InterChunkLayer;
						ChunkData->Chunk->ModifyVoxelsInterChunkLayer(ChunkData->QueuedBlockUpdates);
						ChunkData->QueuedBlockUpdates.Empty();
					}
					else if (ChunkData->GenerationLayer == EGenerationLayer::TerrainLayer)
					{
						ChunkData->Chunk->ClearMesh();
					}
					break;
				}
				// This layer is complete when all chunks have ran updates from the other chunks
				// Here we can initialize final mesh generation of those chunks
				case EGenerationLayer::InterChunkLayer:
				{
					if (ChunkData->HasSixNeighbours && ChunkData->GenerationLayer == EGenerationLayer::InterChunkLayer)
					{
						ChunkData->Chunk->ApplyMesh();
						ChunkData->GenerationLayer = EGenerationLayer::Complete;
					}
					break;
				}
			}
		}
		// Layer expected count should be 0 at this point
	}
		
	//UE_LOG(LogTemp, Warning, TEXT("ChunksCompletedLayerOneGenration: %d"), ChunksCompletedLayerOneGenration);
	//UE_LOG(LogTemp, Warning, TEXT("Total Chunks: %d"), TotalChunks);
}

EBlock AChunkManager::GetBlockFromChunk(const FIntVector& BlockIndex, const FIntVector& ChunkIndex)
{
	return EBlock::Null;
}

bool AChunkManager::GetSixPointers(TSharedPtr<FChunkData> ChunkData)
{
	//UE_LOG(LogTemp, Warning, TEXT("-----------"));
	//UE_LOG(LogTemp, Warning, TEXT("Searching Chunk %d.%d.%d"));
	for (int i = 0; i < 3; i++)
	{
		for (int j = -1; j < 2; j += 2)
		{
			//UE_LOG(LogTemp, Warning, TEXT("j %d"), j);
			FIntVector Direction = FIntVector::ZeroValue;
			Direction[i] += j;
			FIntVector TargetPosition = ChunkData->Position + Direction;

			TSharedPtr<FChunkData>* NeighbourPtr = ChunkMap.Find(TargetPosition);
			TSharedPtr<FChunkData> Neighbour = NeighbourPtr ? *NeighbourPtr : nullptr;
			if (Neighbour)
			{
				//UE_LOG(LogTemp, Warning, TEXT("gp"));
				ChunkData->NeighbourChunks.Add(Neighbour);
			}
			else
			{
				ChunkData->NeighbourChunks.Empty();
				return false;
			}
		}
	}
	ChunkData->HasSixNeighbours = true;
	return true;
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

