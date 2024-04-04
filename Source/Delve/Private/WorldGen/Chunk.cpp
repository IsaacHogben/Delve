// Fill out your copyright notice in the Description page of Project Settings.
#include "Chunk.h"
#include "ProceduralMeshComponent.h"
#include <HAL/RunnableThread.h>

AChunk::AChunk()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");
	Noise = new FastNoiseLite();
	// Mesh Settings
	Mesh->SetCastShadow(true);

	// Set Mesh as root why?
	SetRootComponent(Mesh);
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();

	// Start measuring time
	StartTime = std::chrono::high_resolution_clock::now();

	Noise->SetFrequency(Frequency);
	Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise->SetFractalType(FastNoiseLite::FractalType_FBm);

	Setup();

	// Start async chunk generation and provide the non-thread-safe function as the completion callback
	StartAsyncChunkGen();

}

void AChunk::Setup()
{
	ChunkPosition = GetActorLocation() / WorldScale;
	Blocks.SetNum((ChunkSize + 2) * (ChunkSize + 2) * (ChunkSize + 2));
	BlockSize = WorldScale * Lod;
	//GenerateBlocksFromNoise(ChunkPosition);
}

//
void AChunk::StartAsyncChunkGen()
{
	FGraphEventRef FirstTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		GenerateChunkAsync();
		}, TStatId(), nullptr, ENamedThreads::AnyHiPriThreadHiPriTask);

	FGraphEventRef SecondTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		GenerateChunkAsyncComplete();
		}, TStatId(), FirstTask, ENamedThreads::GameThread);

	UE_LOG(LogTemp, Warning, TEXT("ParentBefore"));
	FGraphEventArray TasksList; 
	TasksList.Add(FirstTask);
	TasksList.Add(SecondTask);
	UE_LOG(LogTemp, Warning, TEXT("ParentAfter"));
}

void AChunk::ModifyVoxelData(const FIntVector Position, const EBlock Block)
{
	const int Index = GetBlockIndex(Position.X, Position.Y, Position.Z);

	Blocks[Index] = Block;
}

void AChunk::GenerateBlocksFromNoise(FVector Position)
{
	for (int x = 0; x < ChunkSize + 2; ++x)
	{
		for (int y = 0; y < ChunkSize + 2; ++y)
		{
			for (int z = 0; z < ChunkSize + 2; ++z)
			{
				const auto NoiseValue = Noise->GetNoise(x + Position.X, y + Position.Y, z + Position.Z);

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

int AChunk::GetBlockIndex(int X, int Y, int Z) const
{
	return Z * (ChunkSize + 2) * (ChunkSize + 2) + Y * (ChunkSize + 2) + X;
}

EBlock AChunk::GetBlock(FIntVector Index, bool checkOutsideChunks)
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

void AChunk::GenerateChunkAsync()
{
	UE_LOG(LogTemp, Warning, TEXT("AsyncStart"));
	GenerateBlocksFromNoise(ChunkPosition);
	if (!IsChunkEmpty)
		GenerateMesh();
}

void AChunk::GenerateChunkAsyncComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("AsyncComplete"));
	if (!IsChunkEmpty)
		ApplyMesh();
	PostStats();
}

void AChunk::GenerateMesh()
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

void AChunk::CreateQuad(
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

bool AChunk::CompareMask(const FMask M1, const FMask M2) const
{
	return M1.Block == M2.Block && M1.Normal == M2.Normal;
}

void AChunk::ApplyMesh() const
{
	//UE_LOG(LogTemp, Warning, TEXT("ApplyMesh"));
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
}

void AChunk::ClearMesh()
{
	VertexCount = 0;
	MeshData.Clear();
}

int AChunk::GetTextureIndex(EBlock Block, FVector Normal) const
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

void AChunk::PostStats()
{
	auto EndTime = std::chrono::high_resolution_clock::now();
	auto Duration = std::chrono::duration_cast<std::chrono::microseconds>(EndTime - StartTime);
	double DurationMilliseconds = Duration.count() / 1000.0;
	double DurationSeconds = Duration.count() / 1000000.0;
	// Log the duration
	UE_LOG(LogTemp, Warning, TEXT("Lod: %d | Vertexes: %d | Execution time: %f milliseconds"), Lod, VertexCount, DurationMilliseconds);
}

void AChunk::ModifyVoxel(const FIntVector Position, const EBlock Block)
{
	if (Position.X >= ChunkSize || Position.Y >= ChunkSize || Position.Z >= ChunkSize || Position.X < 0 || Position.Y < 0 || Position.Z < 0) return;

	ModifyVoxelData(Position, Block);

	ClearMesh();

	GenerateMesh();

	ApplyMesh();
}

void AChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	this->SetActorTickEnabled(false);
}
