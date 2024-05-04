// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkInclude.h"
#include "../Utils/FastNoiseLite.h"

/**
 * 
 */
class ProceduralTerrain
{
public:
	ProceduralTerrain();
	~ProceduralTerrain();

	static EBlock GetBlock(float x, float y, float z, FastNoiseLite* Noise);
	bool IsSurfaceBlock(float Value, float Density);
};
