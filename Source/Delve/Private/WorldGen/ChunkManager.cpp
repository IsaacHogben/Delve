// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGen/ChunkManager.h"

// Sets default values
AChunkManager::AChunkManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AChunkManager::BeginPlay()
{
	Super::BeginPlay();

	//GenerateChunks();
}

void AChunkManager::GenerateChunks()
{
	int s = 8;
	int Lod = 1;
	TArray<int32> LodArray = {1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 8, 16, 32};
	//const auto Chunk = AChunk::StaticClass();
	FIntVector ChunkPos = FIntVector(0, 0, 0);
	for (int z = 0; z < s; z++)
	{
		for (int x = 0; x < s; x++)
		{
			Lod = 0;
			for (int y = 0; y < s; y++)
			{
				ChunkPos = FIntVector(x, y, -z);
				SpawnChunk(ChunkPos, LodArray[Lod]);
				Lod++;
			}
		}
	}
}

void AChunkManager::SpawnChunk(FIntVector ChunkPos, int Lod)
{
	const auto Chunk = AChunk::StaticClass();
	auto transform = FTransform(
		FRotator::ZeroRotator,
		FVector(ChunkPos.X * ChunkSize * 50, ChunkPos.Y * ChunkSize * 50, ChunkPos.Z * ChunkSize * 50),
		FVector::OneVector
	);
	const auto chunk = GetWorld()->SpawnActorDeferred<AChunk>(Chunk, transform, this);
	//do stuff to chunk
	chunk->Lod = Lod;
	chunk->FinishSpawning(transform);
}

// Called every frame
void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	timer++;

	if (timer == 100)//whyewrhjsdfsdfg
		GenerateChunks();

}

