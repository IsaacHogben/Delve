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
	GenerateProceduralTerrain();
}

void UChunkClass::BeginDecoration()
{
	TArray<FCachedBlockUpdate> DecoBlockUpdates = TerrainGenerator->AddDecorationsWithContext(ChunkData->Blocks, this);
	if (DecoBlockUpdates.Num() > 0)
		ModifyVoxels(DecoBlockUpdates, false);
}

// Setup of the chunk class. Only done once
void UChunkClass::Setup()
{
	MeshData = new FChunkMeshData();
	//MeshData2 = new FChunkMeshData();

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
	//PerspectiveMask = CalculatePerspectiveMask(ChunkWorldPosition / ChunkSize);//playerpos

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

	//ClearMeshData();
	//AGenerateMesh();

	//FGraphEventRef CompletionCallback = FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		//UpdateChunkAsyncComplete();
		//}, TStatId(), nullptr, ENamedThreads::GameThread);
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
	//ChunkManager->EnqueueMeshUpdate(Mesh, *MeshData, ChunkWorldPosition, Lod, VertexCount);
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
					const auto CurrentBlockData = ChunkManager->GetBlockData(CurrentBlock);
					const auto CompareBlockData = ChunkManager->GetBlockData(CompareBlock);

					const bool CurrentBlockOpaque = CurrentBlock != EBlock::Air;
					const bool CompareBlockOpaque = CompareBlock != EBlock::Air;

					//Skip this first 'if' if you want to draw Null blocks
					if (!ChunkManager->DrawNullBlocks && (CurrentBlock == EBlock::Null || CompareBlock == EBlock::Null))
					{
						Mask[N++] = FMask{ ChunkManager->GetBlockData(EBlock::Null), 0};
					}
					//Leaves - draw both sides if 2 leaf blocks together
					else if (CurrentBlockData->OpacityMask && CompareBlockData->OpacityMask)
					{
						Mask[N++] = FMask{ CurrentBlockData, 2};
					}
					// - draw block if covered by leaf.
					else if (CurrentBlockData->OpacityMask && CompareBlockOpaque)
					{
						Mask[N++] = FMask{ CompareBlockData, -1};
					}
					// - draw block if covered by leaf (Same but if reverse order)
					else if (CompareBlockData->OpacityMask && CurrentBlockOpaque)
					{
						Mask[N++] = FMask{ CurrentBlockData, 1};
					}
					// Standard Block draw
					else if (CurrentBlockOpaque == CompareBlockOpaque)
					{
						Mask[N++] = FMask{ ChunkManager->GetBlockData(EBlock::Null), 0};
					}
					else if (CurrentBlockOpaque)
					{
						Mask[N++] = FMask{ CurrentBlockData, 1};
					}
					else
					{
						Mask[N++] = FMask{ CompareBlockData, -1};
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
					//const auto Normal = FIntVector(AxisMask * Mask[N].Normal);
					if (Mask[N].Normal != 0)
					{
						
						auto CurrentMask = Mask[N];
						ChunkItr[Axis1] = i;
						ChunkItr[Axis2] = j;

						int Width = 1;
						int Height = 1;

						if (Mask[N].BlockData->GreedyMesh) //check for all other mesh types
						{
							for (Width = 1; i + Width < Axis1Limit && CompareMask(Mask[N + Width], CurrentMask); ++Width)
							{
							}

							bool Done = false;

							for (Height = 1; j + Height < Axis2Limit; ++Height)
							{
								for (int k = 0; k < Width; ++k)
								{
									if (CompareMask(Mask[N + k + Height * Axis1Limit], CurrentMask))			continue;

									Done = true;
									break;
								}

								if (Done) break;
							}
						}

						DeltaAxis1[Axis1] = Width;
						DeltaAxis2[Axis2] = Height;

						// Normal of Two means two sided mesh
						if (CurrentMask.BlockData->IsTwoSided && CurrentMask.Normal == 2)
						{
							CurrentMask.Normal = -1;
							CreateQuad(MeshData, VertexCount,
								CurrentMask, AxisMask, Width, Height,
								FVector(ChunkItr),
								FVector(ChunkItr + DeltaAxis1),
								FVector(ChunkItr + DeltaAxis2),
								FVector(ChunkItr + DeltaAxis1 + DeltaAxis2)
							);
							CurrentMask.Normal = 1;
							CreateQuad(MeshData, VertexCount,
								CurrentMask, AxisMask, Width, Height,
								FVector(ChunkItr),
								FVector(ChunkItr + DeltaAxis1),
								FVector(ChunkItr + DeltaAxis2),
								FVector(ChunkItr + DeltaAxis1 + DeltaAxis2)
							);
						}
						else
							CreateQuad(MeshData, VertexCount, CurrentMask, AxisMask,
								Width,
								Height,
								FVector(ChunkItr),
								FVector(ChunkItr + DeltaAxis1), FVector(ChunkItr + DeltaAxis2), FVector(ChunkItr + DeltaAxis1 + DeltaAxis2));

						DeltaAxis1 = FIntVector::ZeroValue;
						DeltaAxis2 = FIntVector::ZeroValue;

						for (int l = 0; l < Height; ++l)
						{
							for (int k = 0; k < Width; ++k)
							{
								Mask[N + k + l * Axis1Limit] = FMask{ nullptr , 0};
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

void UChunkClass::CreateQuad(FChunkMeshData* meshData, int& vertexCount, const FMask Mask, const FIntVector AxisMask, const int Width, const int Height, const FVector V1,	const FVector V2, const FVector V3, const FVector V4 )
{	
	const auto Normal = FVector(AxisMask * Mask.Normal);

	// Exit Quad creation if blockdata requires.
	if (Mask.BlockData->DisplayFaces == EBlockDisplayType::OnlySides && (Normal.Z != 0))
		return;

	const auto Color = FColor(0, Mask.BlockData->OpacityMask, Mask.BlockData->Face_WPO, GetTextureIndex(Mask.BlockData->Block, Normal));

	meshData->Vertices.Append({
		V1 * BlockSize,
		V2 * BlockSize,
		V3 * BlockSize,
		V4 * BlockSize
		});

	meshData->Triangles.Append({// EXCEPTION_ACCESS_VIOLATION reading address 0xffffffffffffffff
		vertexCount,
		vertexCount + 2 + Mask.Normal,
		vertexCount + 2 - Mask.Normal,
		vertexCount + 3,
		vertexCount + 1 - Mask.Normal,
		vertexCount + 1 + Mask.Normal
		});

	meshData->Normals.Append({
		Normal,
		Normal,
		Normal,
		Normal
		});

	meshData->Colors.Append({
		Color,
		Color,
		Color,
		Color
		});

	if (Normal.X == 1 || Normal.X == -1)
	{
		meshData->UV0.Append({
			FVector2D(Width, Height),
			FVector2D(0, Height),
			FVector2D(Width, 0),
			FVector2D(0, 0),
			});
	}
	else
	{
		meshData->UV0.Append({
			FVector2D(Height, Width),
			FVector2D(Height, 0),
			FVector2D(0, Width),
			FVector2D(0, 0),
			});
	}

	vertexCount += 4;
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
	if (M1.BlockData == nullptr)
		return false;
	return M1.BlockData->Block == M2.BlockData->Block && M1.Normal == M2.Normal;
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
	AGenerateMesh();
	//ChunkData->Blocks.Empty();
	
	AsyncTask(ENamedThreads::GameThread, [this]()
		{
			Mesh = ChunkManager->CreateMeshSection(MeshData, ChunkWorldPosition, VertexCount, Lod, EMeshType::OpaqueCollision);
			VertexCount = 0;
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

	if (DecoBlockUpdates.Num() > 0)
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
			for (const auto& Neighbour : ChunkData->NeighbourChunks)
			{
				if (Neighbour->Position == TargetChunk)
				{
					return Neighbour->Chunk->GetBlock(Index, false);
				}
			}
			// Returns Null if can't find neighbour block - should only occur at the edge of the game world
			return EBlock::Null;
		}
	}
	// Else returns request from within the array
	return ChunkData->Blocks[TerrainGenerator->GetBlockIndex(Index.X, Index.Y, Index.Z)];
}

int UChunkClass::GetTextureIndex(EBlock Block, FVector Normal) const
{
	return static_cast<int>(Block);
	//switch (Block) {
	//case EBlock::Grass:
	//{
	//	//if (Normal == FVector::UpVector) return 0; how to have different faces on one block
	//	return 0;
	//}
	//case EBlock::Dirt: return 2;
	//case EBlock::Stone: return 1;
	//case EBlock::Leaves: return 3;
	//case EBlock::Null: return 4;
	//default: return 255;
	//}
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

// bool OnlyReplaceAir can be changes to enums later for more modify voxel options
FIntVector UChunkClass::ModifyVoxel(FIntVector& Position, const EBlock& Block, bool RegenerateMesh, bool OnlyReplaceAir)
{
	FIntVector RedirectChunk = GetBlockChunkAndIndex(Position);

	if (RedirectChunk == ChunkData->Position)
	{
		if (OnlyReplaceAir && GetBlock(Position, false) == EBlock::Air)
			ModifyVoxelData(Position, Block);
		else if (!OnlyReplaceAir)
			ModifyVoxelData(Position, Block);
		if (RegenerateMesh)
		{
			ApplyMesh();
		}
	}
	return RedirectChunk;
}

// Also handles the redirects so all changes go through here. BlockUpdates should be deleted after being sent here.
void UChunkClass::ModifyVoxels(TArray<FCachedBlockUpdate>& BlockUpdates, bool RegenerateMesh)
{
	TArray<FBlockUpdate> RedirectBlockUpdates;
	FIntVector RedirectChunk;
	for (int i = 0; i < BlockUpdates.Num(); i++)//not seeing any changes from the second update
	{
		RedirectChunk = ModifyVoxel(BlockUpdates[i].Position, BlockUpdates[i].Block, false, true);
		if (RedirectChunk != ChunkData->Position)
		{
			RedirectBlockUpdates.Add(FBlockUpdate(RedirectChunk, BlockUpdates[i].Position, BlockUpdates[i].Block));
		}
	}
	ChunkManager->DistributeBulkChunkUpdates(RedirectBlockUpdates);
	if (RegenerateMesh)
	{
		ApplyMesh();
	}
}

void UChunkClass::TaskGraphDebugLog()
{
	for (const auto Task : TasksList)
	{
		UE_LOG(LogTemp, Warning, TEXT("task %d"), Task.GetReference());
	}
}
