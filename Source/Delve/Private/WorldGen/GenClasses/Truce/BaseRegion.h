// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldGen/GenClasses/LocalRegion.h"


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
    FastNoiseLite* FoliageDensityNoise;

    FTreeSystem Tree1;

    UBaseRegion()
    {
        Noise = new FastNoiseLite(Seed);
        Noise->SetFrequency(0.17);
        Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        Noise->SetFractalType(FastNoiseLite::FractalType_FBm);
        Noise->SetFractalOctaves(5);
        Noise->SetFractalGain(2.75); // Size of subsequent octaves
        Noise->SetFractalLacunarity(0.5); // Density


        FoliageDensityNoise = new FastNoiseLite(Seed);
        FoliageDensityNoise->SetFrequency(0.02);
        FoliageDensityNoise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        FoliageDensityNoise->SetFractalType(FastNoiseLite::FractalType_FBm);
        FoliageDensityNoise->SetFractalOctaves(3);
        FoliageDensityNoise->SetFractalGain(0.5); // Size of subsequent octaves
        FoliageDensityNoise->SetFractalLacunarity(2); // Density
        
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
