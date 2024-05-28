// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGen/ChunkClass.h"
#include "ProceduralMeshComponent.h"
#include <HAL/RunnableThread.h>

UChunkClass::UChunkClass()
{
	//PerspectiveMask = new TArray<FIntVector>{ FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector::ZeroValue };
}

UChunkClass::~UChunkClass()
{
	UE_LOG(LogTemp, Error, TEXT("ChunkClass Deconstructor Called!"));
}

void UChunkClass::BeginGeneration()
{
	Setup();
	GenerateProceduralTerrain();
}

// Setup of the chunk class. Only done once
void UChunkClass::Setup()
{
	MeshData = new FChunkMeshData();

	/*Noise = new FastNoiseLite(999);
	Noise->SetFrequency(Frequency);
	Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise->SetFractalType(FastNoiseLite::FractalType_FBm);*/

	//ChunkWorldPosition = ChunkWorldPosition;
	ChunkData->Position = FIntVector(
		FMath::RoundToInt(ChunkWorldPosition.X),
		FMath::RoundToInt(ChunkWorldPosition.Y),
		FMath::RoundToInt(ChunkWorldPosition.Z)) / ChunkSize;

	BlockSize = WorldScale * Lod;
	PerspectiveMask = CalculatePerspectiveMask(ChunkWorldPosition / ChunkSize);//playerpos
}

void UChunkClass::StartAsyncChunkLodUpdate(int RenderDistance, const float Distance, const FVector PlayerPosition)
{
	FGraphEventRef NewUpdateTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this, RenderDistance, Distance, PlayerPosition]() {
		UpdateChunkLodAsync(RenderDistance, Distance, PlayerPosition);
		}, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);
}

void UChunkClass::StartAsyncChunkPositionUpdate()
{
	bool ContinueToUpdate = true;
	if (!ChunkData.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ChunkData is invalid!"));
	}

	//Update Position
	ChunkWorldPosition = FVector(ChunkData->Position) * ChunkSize;

	//UpdateChunkPositionAsync();
	/*FGraphEventRef NewUpdateTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this, PlayerPosition, NewChunkPosition]() {
		UpdateChunkPositionAsync(PlayerPosition, NewChunkPosition);
		}, TStatId(), nullptr, ENamedThreads::AnyBackgroundHiPriTask);*/

}

void UChunkClass::UpdateChunkLodAsync(int RenderDistance, const float Distance, const FVector PlayerPosition)
{

	if (IsChunkEmpty)
		return;
	bool ContinueToUpdate = false;

	GetLod(RenderDistance, Distance, ContinueToUpdate);
	UpdatePerspectiveMask(PlayerPosition, ContinueToUpdate);

	if (!ContinueToUpdate)
		return;

	ClearMeshData();
	AGenerateMesh();

	FGraphEventRef CompletionCallback = FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		UpdateChunkAsyncComplete();
		}, TStatId(), nullptr, ENamedThreads::GameThread);
}

void UChunkClass::GetLod(int RenderDistance, const float& Distance, bool& ContinueToUpdate)
{
	ChunkRenderDistance crd(RenderDistance);
	//Update Lod if it has changed
	int newLod = crd.CalculateLod(Distance);
	if (Lod != newLod)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Update Lod"));
		Lod = newLod;
		BlockSize = WorldScale * Lod;
		ContinueToUpdate = true;
	}
}

void UChunkClass::UpdatePerspectiveMask(const FVector& PlayerPosition, bool& ContinueToUpdate)
{
	//Skip the mesh Normal mask step for close Lods becuase you can see the shadows missing
	if (Lod != 1)
	{
		TArray<FIntVector> NewPMask = CalculatePerspectiveMask(PlayerPosition);
		for (int i = 0; i < 3; i++)
		{
			if (NewPMask[i] != PerspectiveMask[i])
			{
				PerspectiveMask = NewPMask;
				ContinueToUpdate = true;
			}
		}
	}
}

void UChunkClass::UpdateChunkAsyncComplete()
{
	ChunkManager->EnqueueMeshUpdate(Mesh, *MeshData, ChunkWorldPosition, Lod, VertexCount);
}

// A Prefix implies function is to be run Asynchronously
void UChunkClass::AGenerateMesh()
{
	//UE_LOG(LogTemp, Warning, TEXT("Blocks array accessed. Num elements: %d"), Blocks[99]);

	// Sweep over each axis (X, Y, Z)
	for (int Axis = 0; Axis < 3; ++Axis)
	{
		// 2 Perpendicular axis
		const int Axis1 = (Axis + 1) % 3;
		const int Axis2 = (Axis + 2) % 3;

		const int MainAxisLimit = (ChunkSize) / Lod;
		const int Axis1Limit = (ChunkSize) / Lod;
		const int Axis2Limit = (ChunkSize) / Lod;

		auto DeltaAxis1 = FIntVector::ZeroValue;
		auto DeltaAxis2 = FIntVector::ZeroValue;

		auto ChunkItr = FIntVector::ZeroValue;
		auto AxisMask = FIntVector::ZeroValue;

		AxisMask[Axis] = 1;
		TArray<FMask> Mask;
		Mask.SetNum(Axis1Limit * Axis2Limit);

		// This change prevents overlapping faces on high lod chunks
		int LodModifiedi = 0;
		if (Lod != 1)
			LodModifiedi = -1;

		// Check each slice of the chunk
		for (ChunkItr[Axis] = LodModifiedi; ChunkItr[Axis] < MainAxisLimit;)
		{
			int N = 0;

			// Compute Mask
			for (ChunkItr[Axis2] = 0; ChunkItr[Axis2] < Axis2Limit; ++ChunkItr[Axis2])
			{
				for (ChunkItr[Axis1] = 0; ChunkItr[Axis1] < Axis1Limit; ++ChunkItr[Axis1])
				{
					const auto CurrentBlock = GetBlock(ChunkItr, true);
					const auto CompareBlock = GetBlock(ChunkItr + AxisMask, true);

					const bool CurrentBlockOpaque = CurrentBlock != EBlock::Air;
					const bool CompareBlockOpaque = CompareBlock != EBlock::Air;

					if (CurrentBlockOpaque == CompareBlockOpaque)
					{
						Mask[N++] = FMask{ EBlock::Null, 0 };
					}
					else if (CurrentBlockOpaque)
					{
						Mask[N++] = FMask{ CurrentBlock, 1 };
					}
					else
					{
						Mask[N++] = FMask{ CompareBlock, -1 };
					}
				}
			}

			++ChunkItr[Axis];
			N = 0;

			// Generate Mesh From Mask
			for (int j = 0; j < Axis2Limit; ++j)
			{
				for (int i = 0; i < Axis1Limit;)
				{
					// Bypass faces not facing us
					const auto Normal = FIntVector(AxisMask * Mask[N].Normal);
					if (Mask[N].Normal != 0 && (CompareNormalMask(Normal)))
					{
						const auto CurrentMask = Mask[N];
						ChunkItr[Axis1] = i;
						ChunkItr[Axis2] = j;

						int Width;

						for (Width = 1; i + Width < Axis1Limit && CompareMask(Mask[N + Width], CurrentMask); ++Width)
						{
						}

						int Height;
						bool Done = false;

						for (Height = 1; j + Height < Axis2Limit; ++Height)
						{
							for (int k = 0; k < Width; ++k)
							{
								if (CompareMask(Mask[N + k + Height * Axis1Limit], CurrentMask)) continue;

								Done = true;
								break;
							}

							if (Done) break;
						}

						DeltaAxis1[Axis1] = Width;
						DeltaAxis2[Axis2] = Height;

						CreateQuad(
							CurrentMask, AxisMask, Width, Height,
							FVector(ChunkItr),
							FVector(ChunkItr + DeltaAxis1),
							FVector(ChunkItr + DeltaAxis2),
							FVector(ChunkItr + DeltaAxis1 + DeltaAxis2)
						);

						DeltaAxis1 = FIntVector::ZeroValue;
						DeltaAxis2 = FIntVector::ZeroValue;

						for (int l = 0; l < Height; ++l)
						{
							for (int k = 0; k < Width; ++k)
							{
								Mask[N + k + l * Axis1Limit] = FMask{ EBlock::Null, 0 };
							}
						}

						i += Width;
						N += Width;
					}
					else
					{
						i++;
						N++;
					}
				}
			}
		}
	}
}

void UChunkClass::CreateQuad(const FMask Mask, const FIntVector AxisMask, const int Width, const int Height, const FVector V1, const FVector V2, const FVector V3,	const FVector V4 )
{
	const auto Normal = FVector(AxisMask * Mask.Normal);
	const auto Color = FColor(0, 0, 0, GetTextureIndex(Mask.Block, Normal));

	MeshData->Vertices.Append({ // Modify the Vertex using the noise value at its position to add wonky variation
		V1 * BlockSize,
		V2 * BlockSize,
		V3 * BlockSize,
		V4 * BlockSize
		});

	MeshData->Triangles.Append({
		VertexCount,
		VertexCount + 2 + Mask.Normal,
		VertexCount + 2 - Mask.Normal,
		VertexCount + 3,
		VertexCount + 1 - Mask.Normal,
		VertexCount + 1 + Mask.Normal
		});

	MeshData->Normals.Append({
		Normal,
		Normal,
		Normal,
		Normal
		});

	MeshData->Colors.Append({
		Color,
		Color,
		Color,
		Color
		});

	if (Normal.X == 1 || Normal.X == -1)
	{
		MeshData->UV0.Append({
			FVector2D(Width, Height),
			FVector2D(0, Height),
			FVector2D(Width, 0),
			FVector2D(0, 0),
			});
	}
	else
	{
		MeshData->UV0.Append({
			FVector2D(Height, Width),
			FVector2D(Height, 0),
			FVector2D(0, Width),
			FVector2D(0, 0),
			});
	}

	VertexCount += 4;
}

//Compares the new face normal against the chunks normal mask
bool UChunkClass::CompareNormalMask(FIntVector Normal)
{
	//TArray<FIntVector> MaskArray = PerspectiveMask;
	if (Lod == 1)
		return true;
	if (Normal == PerspectiveMask[0] || Normal == PerspectiveMask[1] || Normal == PerspectiveMask[2])
		return false;
	return true;
}

bool UChunkClass::CompareMask(const FMask M1, const FMask M2) const
{
	return M1.Block == M2.Block && M1.Normal == M2.Normal;
}

TArray<FIntVector> UChunkClass::CalculatePerspectiveMask(FVector PlayerPosition)
{
	//UE_LOG(LogTemp, Warning, TEXT("Calculating perspective mask"));
	const FVector NormalPerspectiveMask = (ChunkWorldPosition / ChunkSize) - PlayerPosition;
	TArray<FIntVector> Mask;

	for (int m = 0; m < 3; m++)
	{
		FIntVector IntVector = FIntVector::ZeroValue;
		if (NormalPerspectiveMask[m] > 0.5)
			IntVector[m] = 1;
		else if (NormalPerspectiveMask[m] < -0.5)
			IntVector[m] = -1;
		Mask.Add(IntVector);
	}
	//UE_LOG(LogTemp, Warning, TEXT("Done Calculating perspective mask"));
	return Mask;
}

void UChunkClass::ApplyMesh()
{
	ClearMeshData();
	AGenerateMesh();
	//Return to game thread

	AsyncTask(ENamedThreads::GameThread, [this]()
		{	
		if (Mesh)
		{
			ChunkManager->EnqueueMeshUpdate(Mesh, *MeshData, ChunkWorldPosition, Lod, VertexCount);
		}
		else
			Mesh = ChunkManager->CreateMeshSection(MeshData, ChunkWorldPosition, VertexCount, Lod);
		});
}

void UChunkClass::ClearMeshData()
{
	VertexCount = 0;
	MeshData->Clear();
}

void UChunkClass::ClearMesh()
{
	if (Mesh)
	{
		ClearMeshData();
		AsyncTask(ENamedThreads::GameThread, [this]()
			{
				ChunkManager->EnqueueMeshUpdate(Mesh, *MeshData, ChunkWorldPosition, Lod, VertexCount);
			});
		
	}
}

void UChunkClass::GenerateProceduralTerrain()
{
	ChunkData->Blocks.SetNum((ChunkSize) * (ChunkSize) * (ChunkSize));
	TArray<FCachedBlockUpdate> DecoBlockUpdates = TerrainGenerator->GetGeneratedChunk(ChunkWorldPosition, ChunkData->Position, ChunkData->Blocks, IsChunkEmpty);

	ModifyVoxels(DecoBlockUpdates, false);
}

EBlock UChunkClass::GetBlock(FIntVector Index, bool checkOutsideChunks)
{
	//Could remove this you want Blocks to be initialized at size equal to Lod.
	Index *= Lod;
	if (checkOutsideChunks)
	{
		FIntVector TargetChunk = GetBlockChunkAndIndex(Index);
		if (TargetChunk != ChunkData->Position)
		{
			//find neighbour by index TODO
			for (const auto& Neighbour : ChunkData->NeighbourChunks)
			{
				if (Neighbour->Position == TargetChunk)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Getting block from neighbour"));
					return Neighbour->Chunk->GetBlock(Index, false);
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("Block Not found. Layer %d. Num Neighbours %d"), ChunkData->GenerationLayer, ChunkData->NeighbourChunks.Num());
		}
	}
	// Else returns request from within the array
	return ChunkData->Blocks[TerrainGenerator->GetBlockIndex(Index.X, Index.Y, Index.Z)];
}

int UChunkClass::GetTextureIndex(EBlock Block, FVector Normal) const
{
	switch (Block) {
	case EBlock::Grass:
	{
		//if (Normal == FVector::UpVector) return 0; how to have different faces on one block
		return 0;
	}
	case EBlock::Dirt: return 2;
	case EBlock::Stone: return 1;
	default: return 255;
	}
}

void UChunkClass::ModifyVoxelData(const FIntVector Position, const EBlock Block)
{
	const int Index = TerrainGenerator->GetBlockIndex(Position.X, Position.Y, Position.Z);

	ChunkData->Blocks[Index] = Block;
}

// Modifies a reference to the block index to represent pos if outside chunk. Returns ChunkPos of new block position
FIntVector UChunkClass::GetBlockChunkAndIndex(FIntVector& Index)
{
	FIntVector RedirectChunk = ChunkData->Position;
	//int BoundsCount = 0;
	for (int i = 0; i < 3; i++)
	{
		while (Index[i] < 0)
		{
			RedirectChunk[i]--;
			Index[i] += ChunkSize;
		}
		while (Index[i] >= ChunkSize)
		{
			RedirectChunk[i]++;
			Index[i] -= ChunkSize;
		}
	}
	return RedirectChunk;
}

//
FIntVector UChunkClass::ModifyVoxel(FIntVector& Position, const EBlock& Block, bool RegenerateMesh)
{
	FIntVector RedirectChunk = GetBlockChunkAndIndex(Position);

	if (RedirectChunk == ChunkData->Position)
	{
		ModifyVoxelData(Position, Block);
		if (RegenerateMesh)
		{
			ClearMeshData();
			AGenerateMesh();
			ApplyMesh();
		}
	}
	return RedirectChunk;
}

// Also handles the redirects so all changes go through here. BlockUpdates should be deleted after being sent here.
void UChunkClass::ModifyVoxels(TArray<FCachedBlockUpdate>& BlockUpdates, bool RegenerateMesh)
{
	//UE_LOG(LogTemp, Warning, TEXT("num negihbs in CC %d %d"), ChunkData->NeighbourChunks.Num(), ChunkData->HasDistributedDecorations);

	TArray<FBlockUpdate> RedirectBlockUpdates;
	FIntVector RedirectChunk;
	for (int i = 0; i < BlockUpdates.Num(); i++)//not seeing any changes from the second update
	{
		RedirectChunk = ModifyVoxel(BlockUpdates[i].Position, BlockUpdates[i].Block, false);
		if (RedirectChunk != ChunkData->Position)
		{
			RedirectBlockUpdates.Add(FBlockUpdate(RedirectChunk, BlockUpdates[i].Position, BlockUpdates[i].Block));
		}
	}
	ChunkManager->DistributeBulkChunkUpdates(RedirectBlockUpdates);
	if (RegenerateMesh)
	{
		ClearMeshData();
		AGenerateMesh();
		Mesh = ChunkManager->CreateMeshSection(MeshData, ChunkWorldPosition, VertexCount, Lod);
	}
}

void UChunkClass::TaskGraphDebugLog()
{
	for (const auto Task : TasksList)
	{
		UE_LOG(LogTemp, Warning, TEXT("task %d"), Task.GetReference());
	}
}
