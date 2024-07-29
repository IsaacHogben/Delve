// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldGen/GenClasses/LocalRegion.h"
#include "Utils/FastNoiseLite.h"

#include "TopRegion.generated.h"

/**
 * 
 */
UCLASS()
class UTopRegion : public ULocalRegion
{
	GENERATED_BODY()
	
public:
    FastNoiseLite* Noise;
    int MaxHeight = 30;

    UTopRegion()
    {
        Topsoil = EBlock::SurfaceGrass;
        Subsoil = EBlock::WhiteStone;
        Bedrock = EBlock::WhiteStone;

        Noise = new FastNoiseLite(Seed);
        Noise->SetFrequency(0.008);
        Noise->SetNoiseType(FastNoiseLite::NoiseType_Cellular);

        Noise->SetFractalType(FastNoiseLite::FractalType_FBm);
        Noise->SetFractalOctaves(2);
        Noise->SetFractalLacunarity(0.5);
        Noise->SetFractalGain(2);

        Noise->SetDomainWarpType(FastNoiseLite::DomainWarpType_BasicGrid);
        Noise->SetDomainWarpAmp(0);

        Noise->SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
        Noise->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Sub);
        Noise->SetCellularJitter(1);
    }

    virtual EBlock GetBlock(ESoilLayer SoilLayer) const override
    {
        // Specific implementation for TopRegion
        return EBlock::Null;
    }
    virtual bool IsInRegion(float& x, float& y, float z) const
    {
        if (z > SurfaceStartHeight)
            return true;
        return false;
    }
private:
    // Region start height
    int RegionStart = 0;
};
