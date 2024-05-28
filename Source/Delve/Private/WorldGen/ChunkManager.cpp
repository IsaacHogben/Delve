// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGen/ChunkManager.h"
//#include "WorldGen/ChunkRenderDistance.h"
FCriticalSection ChunkPositionUpdateLock;
// Sets default values
AChunkManager::AChunkManager()
{
	PreviousPlayerChunkPosition = FIntVector();
	LastUpdateDirection = FIntVector();

	PrimaryActorTick.bCanEverTick = true;
	UpdateProfileTimer = NewObject<UExecutionTimer>();
}

AChunkManager::~AChunkManager()
{
	/*for (UChunkClass* instance : ChunkInstances) {
		delete instance;
	}
	ChunkInstances.clear();*/
	UE_LOG(LogTemp, Error, TEXT("ChunkManager Deconstrusctor Called!"));
	UE_LOG(LogTemp, Error, TEXT("CachedChunkUpdateMap.Num(%d)"), CachedChunkUpdateMap.Num());
}


// Called when the game starts or when spawned
void AChunkManager::BeginPlay()
{
	Super::BeginPlay();
	//Setup
	NoiseManager = NewObject<UNoiseManager>();
	NoiseManager->InitializeArray(GenerationNoiseArray);
	TerrainGenerator = new ProceduralTerrain();
	TerrainGenerator->N = NoiseManager;

	GenerateChunks(PreviousPlayerChunkPosition);
	UpdateChunkGenerationLayerStatus();
}

void AChunkManager::UpdatePlayerChunkPosition(const FVector& PlayerPosition)
{
	UpdatePlayerChunkPositionAsync(PlayerPosition);
}

void AChunkManager::UpdatePlayerChunkPositionAsync(const FVector& PlayerPosition)
{
	UpdateProfileTimer->Start();
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, PlayerPosition]()
		{
			FScopeLock Lock(&ChunkPositionUpdateLock); // Lock here to ensure no race conditions

			UE_LOG(LogTemp, Warning, TEXT("Updating..."));

			bool UpdateDirection = false;
			FIntVector NewPlayerChunkPosition = VectorFunctionUtils::FVectorToFIntVector(PlayerPosition);
			FIntVector Direction = NewPlayerChunkPosition - PreviousPlayerChunkPosition;

			if (Direction != FIntVector(0, 0, 0))
			{
				TArray<FIntVector> AvailablePositions;
				TArray< TSharedPtr<FChunkData>> AvailableChunks;

				for (auto& Elem : ActiveChunkMap)
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
						//ChunkGenerationLayersExpected[int(ECompletedGenerationLayer::InitialTerrainLayer)]++;
					}
					else// Update Lod
					{
						//Broken maybe but only a bit
						// still not worth using due to CreateMeshSection bottleneck
						//ChunkData->Chunk->StartAsyncChunkLodUpdate(RenderDistance, OutChunkDistance, PlayerPosition);
					}
				}

				int i = 0;
				for (auto& ChunkData : AvailableChunks)
				{
					if (i < AvailablePositions.Num())
					{
						FIntVector OldPosition = ChunkData->Position;
						FIntVector NewPosition = AvailablePositions[i];

						//Deactivate Chunk
						InActiveChunkMap.Add(OldPosition, ChunkData);
						if (ChunkData->GenerationLayer == ECompletedGenerationLayer::Complete)
							ChunkData->GenerationLayer = ECompletedGenerationLayer::CompleteInActive;
						ActiveChunkMap.Remove(OldPosition);
						ChunkData->Chunk->ClearMesh();

						//Activate or spawn new chunk
						TSharedPtr<FChunkData>* FoundChunkDataPtr = InActiveChunkMap.Find(NewPosition);
						if (FoundChunkDataPtr != nullptr)
						{
							TSharedPtr<FChunkData> FoundChunkData = *FoundChunkDataPtr;
							FoundChunkData->Chunk = ChunkData->Chunk;
							FoundChunkData->Chunk->ChunkData = FoundChunkData;
							ActiveChunkMap.Add(NewPosition, FoundChunkData);
							AvailableChunks[i] = FoundChunkData;
							//UE_LOG(LogTemp, Warning, TEXT("Chunk found with layer %d"), FoundChunkData->GenerationLayer);
						}
						else //Spawn new chunk
						{
							TSharedPtr<FChunkData> NewChunkData = MakeShared<FChunkData>();
							NewChunkData->Chunk = ChunkData->Chunk;
							NewChunkData->Chunk->ChunkData = NewChunkData;
							NewChunkData->Position = NewPosition;
							ActiveChunkMap.Add(NewPosition, NewChunkData);
							AvailableChunks[i] = NewChunkData;
						}
						//UE_LOG(LogTemp, Warning, TEXT("OldPosition %d.%d.%d\nNewPosition %d.%d.%d\nChunkPosition %d.%d.%d"), OldPosition.X, OldPosition.Y, OldPosition.Z, NewPosition.X, NewPosition.Y, NewPosition.Z, AvailableChunks[i]->Position.X, AvailableChunks[i]->Position.Y, AvailableChunks[i]->Position.Z);
					}				
					i++;
				}
				FThreadSafeCounter Counter(AvailableChunks.Num());
				FEvent* UpdateOperationsCompleteEvent = FPlatformProcess::GetSynchEventFromPool(true);
				for (auto& ChunkData : AvailableChunks)
				{
					if (ChunkData != nullptr)
					{
						//AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, ChunkData, &Counter, UpdateOperationsCompleteEvent]()
							//{
								ChunkData->Chunk->StartAsyncChunkPositionUpdate();
								if (Counter.Decrement() == 0)
								{
									UpdateOperationsCompleteEvent->Trigger();
								}
							//});
					}
				}
				UpdateOperationsCompleteEvent->Wait();
				delete UpdateOperationsCompleteEvent;
				UE_LOG(LogTemp, Warning, TEXT("%f: Chunks Calculated"), UpdateProfileTimer->GetReset());
				UpdateChunkGenerationLayerStatus();
				PreviousPlayerChunkPosition = NewPlayerChunkPosition;

				UE_LOG(LogTemp, Warning, TEXT("%f: Update finished"), UpdateProfileTimer->GetReset());
			}		
		});
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
		TotalChunks++;
	}
	ChunksToSpawn.Empty();
	ChunkGenerationLayersExpected[int(ECompletedGenerationLayer::InitialTerrainLayer)] = TotalChunks;
	UE_LOG(LogTemp, Warning, TEXT("%d Chunks Spawned."), TotalChunks);
}

// Must be done after chunks are spawned, neighbour information used for later generation stages
bool AChunkManager::CheckChunkForNeighbours(TSharedPtr<FChunkData> ChunkData)
{
	return GetSixPointers(ChunkData);
}

void AChunkManager::SpawnChunk(FChunkData data, FIntVector CentralRenderChunkVector)
{
	FIntVector position = (data.Position + CentralRenderChunkVector) * ChunkSize;
	UChunkClass* chunk = NewObject<UChunkClass>();//TODO can move this assignments to the constructor
	chunk->AddToRoot();
	chunk->ChunkManager = this;
	chunk->TerrainGenerator = TerrainGenerator;
	chunk->Lod = data.Lod;
	chunk->ChunkWorldPosition = FVector(position);

	TSharedPtr<FChunkData> ChunkData = MakeShared<FChunkData>();
	ChunkData->Lod = data.Lod;
	ChunkData->Position = data.Position;
	ChunkData->Chunk = chunk;
	chunk->ChunkData = ChunkData;

	ActiveChunkMap.Add(ChunkData->Position, ChunkData);
}

void AChunkManager::StartChunkGeneration()
{
	for (auto& Elem : ActiveChunkMap)
	{
		TSharedPtr<FChunkData> ChunkData = Elem.Value;
		AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, [this, ChunkData]()
			{
				ChunkData->Chunk->BeginGeneration();
			});
	}
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
		Mesh->ClearAllMeshSections();
		return; //EXCEPTION_ACCESS_VIOLATION reading address 11
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
	Mesh->CreateMeshSection( //EXCEPTION_ACCESS_VIOLATION reading address 1
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
	for (int i = 0; i < BlockUpdates.Num(); i++)
	{
		// Try to find the chunk in ChunkMap using the TargetChunk position from BlockUpdates
		TSharedPtr<FChunkData>* FoundChunkData = ActiveChunkMap.Find(BlockUpdates[i].TargetChunk);
		// If cant find in active chunks checks InActive Chunks
		if (FoundChunkData == nullptr)
		{
			FoundChunkData = InActiveChunkMap.Find(BlockUpdates[i].TargetChunk);
		}
		if (FoundChunkData != nullptr)
		{
			// If found, add the block update to the chunk's QueuedBlockUpdates
			FScopeLock Lock(&CriticalQueuedBlockUpdateAddSection);
			(*FoundChunkData)->QueuedBlockUpdates.Add(FCachedBlockUpdate(BlockUpdates[i].Position, BlockUpdates[i].Block));
		}
		else
		{
			TArray<FCachedBlockUpdate>* ChunkUpdateCache = CachedChunkUpdateMap.Find(BlockUpdates[i].TargetChunk);//EXCEPTION_ACCESS_VIOLATION occurred here once
			if (ChunkUpdateCache != nullptr)
			{
				ChunkUpdateCache->Add(FCachedBlockUpdate(BlockUpdates[i].Position, BlockUpdates[i].Block));
			}
			else
			{
				ChunkUpdateCache = &CachedChunkUpdateMap.Add(BlockUpdates[i].TargetChunk);
				ChunkUpdateCache->Add(FCachedBlockUpdate(BlockUpdates[i].Position, BlockUpdates[i].Block));
			}
		}
	}
}

// Chunks send their current generation status here when it is complete to sync layers. All GenerationLayer changes go through here for continuity
void AChunkManager::UpdateChunkGenerationLayerStatus()
{
	TArray<TSharedPtr<FChunkData>> ChunkTrickleDownGenerationList;
	FThreadSafeCounter Counter(TotalChunks);
	FEvent* AllOperationsCompleteEvent = FPlatformProcess::GetSynchEventFromPool(true);

	FCriticalSection CriticalListSection;

	UE_LOG(LogTemp, Warning, TEXT("%f:Initial Generation of new Chunks"), UpdateProfileTimer->GetReset());
	// Initial Generation
	for (auto& Elem : ActiveChunkMap)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, Elem, AllOperationsCompleteEvent, &Counter]()
			{
				//FPlatformProcess::Sleep(2.0f); // Simulate an async operation
				TSharedPtr<FChunkData> ChunkData = Elem.Value;
				if (!ChunkData.IsValid())
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid ChunkData"));
					return;
				}
				if (ChunkData->GenerationLayer == ECompletedGenerationLayer::Empty)
				{
					//UE_LOG(LogTemp, Warning, TEXT("EmptyChunkIniialized"));
					ChunkData->Chunk->BeginGeneration();
					ChunkData->GenerationLayer = ECompletedGenerationLayer::InitialTerrainLayer;
				}
				if (Counter.Decrement() == 0)
				{
					AllOperationsCompleteEvent->Trigger();
				}
			});
	}
	AllOperationsCompleteEvent->Wait();
	AllOperationsCompleteEvent->Reset();

	Counter.Set(TotalChunks);

	UE_LOG(LogTemp, Warning, TEXT("%f:Selecting Chunks to Update"), UpdateProfileTimer->GetReset());
	for (auto& Elem : ActiveChunkMap)
	{
		//AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, Elem, &ChunkTrickleDownGenerationList, AllOperationsCompleteEvent, &Counter, &CriticalListSection]()
			//{
				TSharedPtr<FChunkData> ChunkData = Elem.Value;
				if (ChunkData->GenerationLayer == ECompletedGenerationLayer::InitialTerrainLayer && CheckChunkForNeighbours(ChunkData))
				{
					FScopeLock ListLock(&CriticalListSection);
					ChunkData->GenerationLayer = ECompletedGenerationLayer::HasNeigboursLayer;
					ChunkTrickleDownGenerationList.Add(ChunkData);
				}
				// Slip in the previously completed chunks to the list to be applied later
				else if (ChunkData->GenerationLayer == ECompletedGenerationLayer::CompleteInActive && CheckChunkForNeighbours(ChunkData))
				{
					FScopeLock ListLock(&CriticalListSection);
					ChunkTrickleDownGenerationList.Add(ChunkData);
				}
				// Add Active chunks with new block data reaching over from neighbour chunks
				else if (ChunkData->GenerationLayer == ECompletedGenerationLayer::Complete && ChunkData->NeighbourChunks.Num() == 6 && ChunkData->QueuedBlockUpdates.Num() > 0)
				{
					FScopeLock ListLock(&CriticalListSection);
					ChunkTrickleDownGenerationList.Add(ChunkData);
				}

				if (Counter.Decrement() == 0)
				{
					AllOperationsCompleteEvent->Trigger();
				}
			//});
	}
	AllOperationsCompleteEvent->Wait();
	AllOperationsCompleteEvent->Reset();

	UE_LOG(LogTemp, Warning, TEXT("%f:Found %d Chunks to Update"), UpdateProfileTimer->GetReset(), ChunkTrickleDownGenerationList.Num());
	if (ChunkTrickleDownGenerationList.Num() == 0)
	{
		delete AllOperationsCompleteEvent;
		return;
	}

	Counter.Set(ChunkTrickleDownGenerationList.Num());
	UE_LOG(LogTemp, Warning, TEXT("Updating Chunks"));
	for (auto& ChunkData : ChunkTrickleDownGenerationList)
	{
		//data test can remove in clean
		if (!ChunkData.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid ChunkData"));
			UE_LOG(LogTemp, Error, TEXT("With layer %d"), ChunkData->GenerationLayer);
		}
		//AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, ChunkData, AllOperationsCompleteEvent, &Counter]()
			//{
				if (ChunkData->GenerationLayer == ECompletedGenerationLayer::HasNeigboursLayer || ChunkData->GenerationLayer == ECompletedGenerationLayer::Complete)
				{
					StartDecorationApplication(ChunkData);
					ChunkData->GenerationLayer = ECompletedGenerationLayer::DecorationLayer;
				}
				if (Counter.Decrement() == 0)
				{
					AllOperationsCompleteEvent->Trigger();
				}
			//});
	}
	AllOperationsCompleteEvent->Wait();
	AllOperationsCompleteEvent->Reset();

	Counter.Set(ChunkTrickleDownGenerationList.Num());

	UE_LOG(LogTemp, Warning, TEXT("%f:Applying Meshes"), UpdateProfileTimer->GetReset());
	for (auto& ChunkData : ChunkTrickleDownGenerationList)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, ChunkData, AllOperationsCompleteEvent, &Counter]()
			{
				ChunkData->Chunk->ApplyMesh();
				ChunkData->GenerationLayer = ECompletedGenerationLayer::Complete;
				if (Counter.Decrement() == 0)
				{
					AllOperationsCompleteEvent->Trigger();
				}
			});
	}
	AllOperationsCompleteEvent->Wait();
	AllOperationsCompleteEvent->Reset();

	Counter.Set(ChunkTrickleDownGenerationList.Num());

	UE_LOG(LogTemp, Warning, TEXT("%f:Performing CleanUp"), UpdateProfileTimer->GetReset());
	for (auto& ChunkData : ChunkTrickleDownGenerationList)
	{
		//AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, ChunkData, AllOperationsCompleteEvent, &Counter]()
			//{
				CleanUpCachedData(ChunkData);
				if (Counter.Decrement() == 0)
				{
					AllOperationsCompleteEvent->Trigger();
				}
			//});
	}
	AllOperationsCompleteEvent->Wait();
	AllOperationsCompleteEvent->Reset();

	UE_LOG(LogTemp, Warning, TEXT("%f:Completed %d Chunk Updates"), UpdateProfileTimer->GetReset(), ChunkTrickleDownGenerationList.Num());

	// Cleanup
	delete AllOperationsCompleteEvent;
}

void AChunkManager::StartDecorationApplication(TSharedPtr<FChunkData> ChunkData)
{
	//Add any cached updates
	TArray<FCachedBlockUpdate>* ChunkUpdateCached = CachedChunkUpdateMap.Find(ChunkData->Position);
	if (ChunkUpdateCached != nullptr)
	{
		TArray<FCachedBlockUpdate> UpdatesToAppend = *ChunkUpdateCached;
		ChunkData->QueuedBlockUpdates.Append(UpdatesToAppend);
		//CachedChunkUpdateMap.Remove(ChunkData->Position);
	}
	// Do application operation
	//if (ChunkData->QueuedBlockUpdates.Num() > 0)
	{
		ChunkData->Chunk->ModifyVoxels(ChunkData->QueuedBlockUpdates, false);
		ChunkData->QueuedBlockUpdates.Empty();
		ChunkData->HasDistributedDecorations = true;
	}
}

EBlock AChunkManager::GetBlockFromChunk(const FIntVector& BlockIndex, const FIntVector& ChunkIndex)
{
	return EBlock::Null;
}

bool AChunkManager::GetSixPointers(TSharedPtr<FChunkData> ChunkData)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = -1; j < 2; j += 2)
		{
			FIntVector Direction = FIntVector::ZeroValue;
			Direction[i] += j;
			FIntVector TargetPosition = ChunkData->Position + Direction;

			TSharedPtr<FChunkData>* NeighbourPtr = ActiveChunkMap.Find(TargetPosition);
			TSharedPtr<FChunkData> Neighbour = NeighbourPtr ? *NeighbourPtr : nullptr;
			if (Neighbour)
			{
				ChunkData->NeighbourChunks.Add(Neighbour);
			}
			else
			{
				ChunkData->NeighbourChunks.Empty();
				return false;
			}
		}
	}
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

void AChunkManager::CleanUpCachedData(TSharedPtr<FChunkData> ChunkData)
{
	TArray<FCachedBlockUpdate>* ChunkUpdateCached = CachedChunkUpdateMap.Find(ChunkData->Position);
	if (ChunkUpdateCached != nullptr)
	{
		TArray<FCachedBlockUpdate> UpdatesToClean = *ChunkUpdateCached;
		if (UpdatesToClean.Num() == 0)
			CachedChunkUpdateMap.Remove(ChunkData->Position);
	}
}

