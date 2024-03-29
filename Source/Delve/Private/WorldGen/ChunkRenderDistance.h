// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utils/Enums.h"

/**
 * 
 */
class ChunkRenderDistance
{
public:
	ChunkRenderDistance();
	void CalculateRenderSphere();
	~ChunkRenderDistance();
private:
	int DrawDistance = 5;
	TArray<TArray<FIntVector>> RenderHemisphere;
};
