// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGen/ProceduralTerrain.h"

ProceduralTerrain::ProceduralTerrain()
{
}

ProceduralTerrain::~ProceduralTerrain()
{
}

EBlock ProceduralTerrain::GetBlock(float x, float y, float z, FastNoiseLite* Noise)
{
	const auto Value = Noise->GetNoise(x, y, z);
	const auto UpValue = Noise->GetNoise(x, y, z + 1);

	const float Density = 0;

	if (Value >= Density)
	{
		return EBlock::Air;
	}
	else //Not Air
	{
		if (UpValue >= Density)
			return EBlock::Grass;
		if (UpValue > Value)
		{ 
			if (Value >= (Density - 0.0024f))
				return EBlock::Grass;
			if (Value >= (Density - 0.016f))
				return EBlock::Dirt;
		}
		return EBlock::Stone;
	}
}

bool ProceduralTerrain::IsSurfaceBlock(float UpValue, float Density)
{
	if (UpValue >= Density)
		return true;
	return false;
}
