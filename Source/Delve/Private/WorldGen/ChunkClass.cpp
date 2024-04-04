// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGen/ChunkClass.h"
#include "ProceduralMeshComponent.h"
#include <HAL/RunnableThread.h>

ChunkClass::ChunkClass()
{
	
}

// Called when the game starts or when spawned
void ChunkClass::BeginPlay()
{
	StartAsyncChunkGen();
}

void ChunkClass::RenderDistanceUpdate(const FVector& Position, int RenderDistance)
{	
	//UE_LOG(LogTemp, Warning, TEXT("oh lordy"));
	StartAsyncChunkUpdate(Position, RenderDistance);
}

void ChunkClass::Setup()
{
	Noise = new FastNoiseLite();
	Noise->SetFrequency(Frequency);
	Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise->SetFractalType(FastNoiseLite::FractalType_FBm);

	ChunkPosition = ChunkPosition / WorldScale;
	ChunkVector = FIntVector(
		FMath::RoundToInt(ChunkPosition.X),
		FMath::RoundToInt(ChunkPosition.Y),
		FMath::RoundToInt(ChunkPosition.Z) / 32
	);
	Blocks.SetNum((ChunkSize + 2) * (ChunkSize + 2) * (ChunkSize + 2));
	BlockSize = WorldScale * Lod;
}

void ChunkClass::StartAsyncChunkGen()
{
	FGraphEventRef FirstTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		GenerateChunkAsync();
		}, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);

	FGraphEventRef SecondTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		GenerateChunkAsyncComplete();
		}, TStatId(), FirstTask, ENamedThreads::GameThread);

	FGraphEventArray TasksList;
	TasksList.Add(FirstTask);
}

void ChunkClass::StartAsyncChunkUpdate(const FVector& Position, int RenderDistance)
{
	FGraphEventRef FirstTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this, Position, RenderDistance]() {
		UpdateChunkAsync(Position, RenderDistance);
		}, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);

	FGraphEventRef SecondTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		UpdateChunkAsyncComplete();
		}, TStatId(), FirstTask, ENamedThreads::GameThread);

	FGraphEventArray TasksList;
	TasksList.Add(FirstTask);

}

void ChunkClass::ModifyVoxelData(const FIntVector Position, const EBlock Block)
{
	const int Index = GetBlockIndex(Position.X, Position.Y, Position.Z);

	Blocks[Index] = Block;
}

void ChunkClass::GenerateBlocksFromNoise(FVector Position)
{
	for (int x = 0; x < ChunkSize + 2; ++x)
	{
		for (int y = 0; y < ChunkSize + 2; ++y)
		{
			for (int z = 0; z < ChunkSize + 2; ++z)
			{
				const auto NoiseValue = Noise->GetNoise(x + Position.X, y + Position.Y, z + Position.Z);

				if (NoiseValue >= 0 || z + Position.Z > -4)
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

int ChunkClass::GetBlockIndex(int X, int Y, int Z) const
{
	return Z * (ChunkSize + 2) * (ChunkSize + 2) + Y * (ChunkSize + 2) + X;
}

EBlock ChunkClass::GetBlock(FIntVector Index, bool checkOutsideChunks)
{

	//Could remove this you want Blocks to be initialized at size equal to Lod.
	Index *= Lod;
	Index += FIntVector(1, 1, 1);
	//Checks out of bounds of the Chunk
	if (Index.X >= ChunkSize + 2 || Index.Y >= ChunkSize + 2 || Index.Z >= ChunkSize + 2 || Index.X < 0 || Index.Y < 0 || Index.Z < 0)
	{
		if (!checkOutsideChunks)
		{
			if (Lod >= 8)//shows all faces for low lod
				return EBlock::Air;
			if (Index.X >= ChunkSize + 2)//returns chunk padding for less inter-chunk referencing
				Index.X = ChunkSize + 1;
			else if (Index.X < 0)
				Index.X = 0;
			if (Index.Z >= ChunkSize + 2)
				Index.Z = ChunkSize + 1;
			else if (Index.Y < 0)
				Index.Y = 0;
			if (Index.Z >= ChunkSize + 2)
				Index.Z = ChunkSize + 1;
			else if (Index.Z < 0)
				Index.Z = 0;
		}
		else
			return EBlock::Air; //IE check for block in another chunk TODO
	}
	return Blocks[GetBlockIndex(Index.X, Index.Y, Index.Z)];
}

void ChunkClass::GenerateChunkAsync()
{
	//UE_LOG(LogTemp, Warning, TEXT("AsyncStartCount"));
	//std::this_thread::sleep_for(std::chrono::seconds(6));
	//UE_LOG(LogTemp, Warning, TEXT("AsyncEndCount"));
	Setup();
	GenerateBlocksFromNoise(ChunkPosition);
	if (!IsChunkEmpty)
		GenerateMesh();
}

void ChunkClass::GenerateChunkAsyncComplete()
{
	if (!IsChunkEmpty)
		Mesh = ChunkManager->CreateMeshSection(MeshData, ChunkPosition, VertexCount, Lod);
}

void ChunkClass::UpdateChunkAsync(const FVector& Position, int RenderDistance)
{
	ChunkRenderDistance crd(RenderDistance);
	float distance = FMath::Sqrt(FVector::DistSquared(Position, FVector(ChunkVector) / ChunkSize));
	//UE_LOG(LogTemp, Warning, TEXT("pos > %f"), distance);
	if (distance >= RenderDistance)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Out"));
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("In"));
	int newLod = crd.CalculateLod(distance);
	if (Lod == newLod)
		return;
	Lod = newLod;
	BlockSize = WorldScale * Lod;
	ClearMeshData();
	if (!IsChunkEmpty)
		GenerateMesh();
}

void ChunkClass::UpdateChunkAsyncComplete()
{
	//UE_LOG(LogTemp, Warning, TEXT("UpdateS>"));
	//std::chrono::high_resolution_clock::time_point StartTime = std::chrono::high_resolution_clock::now();
	if (!IsChunkEmpty)
		ChunkManager->UpdateMeshSection(Mesh, MeshData, ChunkPosition, Lod);
	//PostStats(StartTime);
}

void ChunkClass::GenerateMesh()
{
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

		// Check each slice of the chunk
		for (ChunkItr[Axis] = -1; ChunkItr[Axis] < MainAxisLimit;)
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
					if (Mask[N].Normal != 0)
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
							ChunkItr,
							ChunkItr + DeltaAxis1,
							ChunkItr + DeltaAxis2,
							ChunkItr + DeltaAxis1 + DeltaAxis2
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

void ChunkClass::CreateQuad(
	const FMask Mask,
	const FIntVector AxisMask,
	const int Width,
	const int Height,
	const FIntVector V1,
	const FIntVector V2,
	const FIntVector V3,
	const FIntVector V4
)
{
	const auto Normal = FVector(AxisMask * Mask.Normal);
	const auto Color = FColor(0, 0, 0, GetTextureIndex(Mask.Block, Normal));

	MeshData.Vertices.Append({
		FVector(V1) * BlockSize,
		FVector(V2) * BlockSize,
		FVector(V3) * BlockSize,
		FVector(V4) * BlockSize
		});

	MeshData.Triangles.Append({
		VertexCount,
		VertexCount + 2 + Mask.Normal,
		VertexCount + 2 - Mask.Normal,
		VertexCount + 3,
		VertexCount + 1 - Mask.Normal,
		VertexCount + 1 + Mask.Normal
		});

	MeshData.Normals.Append({
		Normal,
		Normal,
		Normal,
		Normal
		});

	MeshData.Colors.Append({
		Color,
		Color,
		Color,
		Color
		});

	if (Normal.X == 1 || Normal.X == -1)
	{
		MeshData.UV0.Append({
			FVector2D(Width, Height),
			FVector2D(0, Height),
			FVector2D(Width, 0),
			FVector2D(0, 0),
			});
	}
	else
	{
		MeshData.UV0.Append({
			FVector2D(Height, Width),
			FVector2D(Height, 0),
			FVector2D(0, Width),
			FVector2D(0, 0),
			});
	}

	VertexCount += 4;
}

bool ChunkClass::CompareMask(const FMask M1, const FMask M2) const
{
	return M1.Block == M2.Block && M1.Normal == M2.Normal;
}

void ChunkClass::ApplyMesh()
{
	//Mesh = ChunkManager->CreateMeshSection(MeshData, ChunkPosition, VertexCount, Lod);
}

void ChunkClass::ClearMeshData()
{
	VertexCount = 0;
	MeshData.Clear();
}

int ChunkClass::GetTextureIndex(EBlock Block, FVector Normal) const
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

void ChunkClass::PostStats(std::chrono::high_resolution_clock::time_point StartTime)
{
	auto EndTime = std::chrono::high_resolution_clock::now();
	auto Duration = std::chrono::duration_cast<std::chrono::microseconds>(EndTime - StartTime);
	double DurationMilliseconds = Duration.count() / 1000.0;
	double DurationSeconds = Duration.count() / 1000000.0;
	// Log the duration
	UE_LOG(LogTemp, Warning, TEXT("Lod: %d | Vertexes: %d | Execution time: %f milliseconds"), Lod, VertexCount, DurationMilliseconds);
}

void ChunkClass::ModifyVoxel(const FIntVector Position, const EBlock Block)
{
	if (Position.X >= ChunkSize || Position.Y >= ChunkSize || Position.Z >= ChunkSize || Position.X < 0 || Position.Y < 0 || Position.Z < 0) return;

	ModifyVoxelData(Position, Block);

	ClearMeshData();

	GenerateMesh();

	ApplyMesh();
}

