// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "ProceduralMeshComponent.h"
#include "Templates/UniquePtr.h"

#include "../Utils/Enums.h"
#include "../Utils/FastNoiseLite.h"
#include "../Utils/ChunkMeshData.h"
#include "../Utils/BenchmarkTimer.h"
#include "../Utils/VectorFunctionUtils.h"
#include "../Utils/ChunkStructs.h"
#include "../Utils/ExecutionTimer.h"

#include "ChunkManager.h"
#include "ChunkClass.h"


/**
 * 
 */
class ChunkInclude
{
public:
	ChunkInclude();
	~ChunkInclude();
};
