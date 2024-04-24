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

// Called when the game starts or when spawned
void UChunkClass::BeginPlay()
{
	StartAsyncChunkGen(FVector::Zero());
}

//void UChunkClass::ChunkPositionUpdate(const FVector PlayerPosition, const FIntVector NewChunkPosition)
//{
//	UE_LOG(LogTemp, Warning, TEXT("ChunkClass PosUp Called"));
//}
//
//void UChunkClass::ChunkLodUpdate(int RenderDistance, const float Distance, const FVector PlayerPosition)
//{	
//	//UE_LOG(LogTemp, Warning, TEXT("oh lordy"));
//	StartAsyncChunkLodUpdate(RenderDistance, Distance, PlayerPosition);
//}

void UChunkClass::Setup()
{
	Blocks.SetNum((ChunkSize + 2) * (ChunkSize + 2) * (ChunkSize + 2));
	MeshData = new FChunkMeshData();

	Noise = new FastNoiseLite(33253);
	Noise->SetFrequency(Frequency);
	Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise->SetFractalType(FastNoiseLite::FractalType_FBm);

	//ChunkWorldPosition = ChunkWorldPosition;
	ChunkVectorPosition = FIntVector(
		FMath::RoundToInt(ChunkWorldPosition.X),
		FMath::RoundToInt(ChunkWorldPosition.Y),
		FMath::RoundToInt(ChunkWorldPosition.Z)) / ChunkSize;

	BlockSize = WorldScale * Lod;
	PerspectiveMask = CalculatePerspectiveMask(ChunkWorldPosition / ChunkSize);//playerpos
}

void UChunkClass::StartAsyncChunkGen(const FVector& PlayerPosition)
{
	FGraphEventRef FirstTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this, PlayerPosition]() {
		GenerateChunkAsync(PlayerPosition);
		}, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);

	FGraphEventRef SecondTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		GenerateChunkAsyncComplete();
		}, TStatId(), FirstTask, ENamedThreads::GameThread);

	//FGraphEventArray TasksList;
	TasksList.Add(FirstTask);
}

void UChunkClass::StartAsyncChunkLodUpdate(int RenderDistance, const float Distance, const FVector PlayerPosition)
{
	FGraphEventRef NewUpdateTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this, RenderDistance, Distance, PlayerPosition]() {
		UpdateChunkLodAsync(RenderDistance, Distance, PlayerPosition);
		}, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);

	// Override any outstanding task
	if (TasksList.Contains(PreviousLodUpdateTask))
	{
		TasksList.Remove(PreviousLodUpdateTask);
	}

	TasksList.Add(NewUpdateTask);
	PreviousLodUpdateTask = NewUpdateTask;
}

void UChunkClass::StartAsyncChunkPositionUpdate(const FVector PlayerPosition, const FIntVector NewChunkPosition)
{
	FGraphEventRef NewUpdateTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this, PlayerPosition, NewChunkPosition]() {
		UpdateChunkPositionAsync(PlayerPosition, NewChunkPosition);
		}, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);

	TasksList.Add(NewUpdateTask);
}

void UChunkClass::ModifyVoxelData(const FIntVector Position, const EBlock Block)
{
	const int Index = GetBlockIndex(Position.X, Position.Y, Position.Z);

	Blocks[Index] = Block;
}

void UChunkClass::GenerateBlocksFromNoise(FVector Position)
{
	FastNoiseLite CellNoise = FastNoiseLite(33253);
	CellNoise.SetFrequency(Frequency);
	CellNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	CellNoise.SetFractalType(FastNoiseLite::FractalType_None);
	CellNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Div);
	CellNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Manhattan);

	for (int x = 0; x < ChunkSize + 2; ++x)
	{
		for (int y = 0; y < ChunkSize + 2; ++y)
		{
			for (int z = 0; z < ChunkSize + 2; ++z)
			{
				const auto NoiseValue = Noise->GetNoise(x + Position.X, y + Position.Y, z + Position.Z);
				const auto CellNoiseValue = CellNoise.GetNoise(x + Position.X, y + Position.Y, 0.0);

				//if (z + Position.Z > CellNoiseValue * 64)
					//Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
				if (NoiseValue >= 0)
				{
					Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
				}
				else
				{
					Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
					IsChunkEmpty = false;
				}
			}
		}
	}
}

int UChunkClass::GetBlockIndex(int X, int Y, int Z) const
{
	return Z * (ChunkSize + 2) * (ChunkSize + 2) + Y * (ChunkSize + 2) + X;
}

EBlock UChunkClass::GetBlock(FIntVector Index, bool checkOutsideChunks)
{

	//Could remove this you want Blocks to be initialized at size equal to Lod.
	Index *= Lod;
	Index += FIntVector(1, 1, 1);

	//Manages requests for blocks that are outside of the array
	// Was changed from ChunkSize + 2 to fix holes in higher LODS // TODO investigate impact on performance further
	if (Index.X >= ChunkSize + 1 || Index.Y >= ChunkSize + 1 || Index.Z >= ChunkSize + 1 || Index.X <= 0 || Index.Y <= 0 || Index.Z <= 0)
	{
		if (!checkOutsideChunks) //Used for checks that dont break thread saftey
		{		
			for (int i = 0; i < 3; i++)
			{
				if (Index[i] >= ChunkSize + 2)
					Index[i] = ChunkSize + 1;
				else if (Index[i] < 0)
					Index[i] = 0;
			}
			if (Lod > 1) //Filld Gaps in low LOD Chunks
				return EBlock::Air;
		}
		else// IE check for block in another chunk TODO	
		{
			UE_LOG(LogTemp, Error, TEXT("Attempting to read blocks outside chunk"));
			return EBlock::Air; 
		}
	}
	// Else returns request from within the array
	return Blocks[GetBlockIndex(Index.X, Index.Y, Index.Z)];
}

void UChunkClass::GenerateChunkAsync(const FVector& PlayerPosition)
{
	Setup();
	GenerateBlocksFromNoise(ChunkWorldPosition);
	if (!IsChunkEmpty)
		GenerateMesh();
}

void UChunkClass::GenerateChunkAsyncComplete()
{
	Mesh = ChunkManager->CreateMeshSection(MeshData, ChunkWorldPosition, VertexCount, Lod);
}

void UChunkClass::UpdateChunkLodAsync(int RenderDistance, const float Distance, const FVector PlayerPosition)
{
	//God Code do not Touch
	std::this_thread::sleep_for(std::chrono::nanoseconds(Lod - 1));

	//Setup
	bool ContinueToUpdate = false;

	GetLod(RenderDistance, Distance, ContinueToUpdate);
	UpdatePerspectiveMask(PlayerPosition, ContinueToUpdate);

	if (!ContinueToUpdate)
		return;
	
	ClearMeshData();
	GenerateMesh();

	FGraphEventRef CompletionCallback = FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		UpdateChunkAsyncComplete();
		}, TStatId(), nullptr, ENamedThreads::GameThread);

	//FGraphEventArray TasksList;
	TasksList.Add(CompletionCallback);
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

void UChunkClass::UpdateChunkPositionAsync(const FVector PlayerPosition, const FIntVector NewChunkPosition)
{
	bool ContinueToUpdate = true;

	//Update Position
	ChunkVectorPosition = NewChunkPosition;
	ChunkWorldPosition = FVector(ChunkVectorPosition) * ChunkSize;

	//Get Blocks for new position
	GenerateBlocksFromNoise(ChunkWorldPosition);

	UpdatePerspectiveMask(PlayerPosition, ContinueToUpdate);

	ClearMeshData();
	GenerateMesh();

	FGraphEventRef CompletionCallback = FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		UpdateChunkAsyncComplete();
		}, TStatId(), nullptr, ENamedThreads::GameThread);

	//FGraphEventArray TasksList;
	TasksList.Add(CompletionCallback);
}

void UChunkClass::UpdateChunkAsyncComplete()
{
	ChunkManager->UpdateMeshSection(Mesh, MeshData, ChunkWorldPosition, Lod, VertexCount);
}

void UChunkClass::GenerateMesh()
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
					const auto CurrentBlock = GetBlock(ChunkItr, 0);
					const auto CompareBlock = GetBlock(ChunkItr + AxisMask, 0);

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

void UChunkClass::CreateQuad(
	const FMask Mask,
	const FIntVector AxisMask,
	const int Width,
	const int Height,
	const FVector V1,
	const FVector V2,
	const FVector V3,
	const FVector V4
)
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
	//Mesh = ChunkManager->CreateMeshSectiosssswn(MeshData, ChunkPosition, VertexCount, Lod);
}

void UChunkClass::ClearMeshData()
{
	VertexCount = 0;
	MeshData->Clear();
}

int UChunkClass::GetTextureIndex(EBlock Block, FVector Normal) const
{
	switch (Block) {
	case EBlock::Grass:
	{
		if (Normal == FVector::UpVector) return 0;
		return 1;
	}
	case EBlock::Dirt: return 2;
	case EBlock::Stone: return 3;
	default: return 255;
	}
}

void UChunkClass::PostStats(std::chrono::high_resolution_clock::time_point StartTime)
{
	auto EndTime = std::chrono::high_resolution_clock::now();
	auto Duration = std::chrono::duration_cast<std::chrono::microseconds>(EndTime - StartTime);
	double DurationMilliseconds = Duration.count() / 1000.0;
	double DurationSeconds = Duration.count() / 1000000.0;
	// Log the duration
	UE_LOG(LogTemp, Warning, TEXT("Lod: %d | Vertexes: %d | Execution time: %f milliseconds"), Lod, VertexCount, DurationMilliseconds);
}

void UChunkClass::ModifyVoxel(const FIntVector Position, const EBlock Block)
{
	if (Position.X >= ChunkSize || Position.Y >= ChunkSize || Position.Z >= ChunkSize || Position.X < 0 || Position.Y < 0 || Position.Z < 0) return;

	ModifyVoxelData(Position, Block);

	ClearMeshData();

	//GenerateMesh(PlayerPosition);

	ApplyMesh();
}

