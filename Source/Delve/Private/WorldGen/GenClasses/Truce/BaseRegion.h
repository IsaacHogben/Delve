// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldGen/GenClasses/LocalRegion.h"
#include "Utils/FastNoiseLite.h"

#include "BaseRegion.generated.h"

/**
 * 
 */
UCLASS()
class UBaseRegion : public ULocalRegion
{
	GENERATED_BODY()
	
public:
    FastNoiseLite* Noise;

    UBaseRegion()
    {
        Noise = new FastNoiseLite(Seed);
        Noise->SetFrequency(0.05);
        Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);

        Noise->SetFractalType(FastNoiseLite::FractalType_FBm);
        Noise->SetFractalOctaves(3);
        Noise->SetFractalGain(2); // Size of subsequent octaves
        Noise->SetFractalLacunarity(0.5); // Density
        
    };

    virtual EBlock GetBlock(ESoilLayer SoilLayer) const override
    {
        // Specific implementation for Base region

        return EBlock::Null;
    }
    bool IsInRegion(float& x, float& y, float& z) const
    {
        // Specific implementation for Base region

        return true;
    }
};
