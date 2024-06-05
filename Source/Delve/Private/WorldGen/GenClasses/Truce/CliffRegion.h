// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldGen/GenClasses/LocalRegion.h"
#include "Utils/FastNoiseLite.h"

#include "CliffRegion.generated.h"

/**
 * 
 */
UCLASS()
class UCliffRegion : public ULocalRegion
{
    GENERATED_BODY()

public:
    FastNoiseLite* Noise;

    UCliffRegion()
    {
        Topsoil = EBlock::Moss;
        Subsoil = EBlock::Stone;
        Bedrock = EBlock::CliffStone;

        Noise = new FastNoiseLite();
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
        // Specific implementation for Base region
        return EBlock::Null;
    }
    virtual bool IsInRegion(float& x, float& y, float& z) const override
    {
        // Specific implementation for Base region
        if (z > RegionEnd && z < RegionStart)
        {
            if (Noise->GetNoise(x, y, z) < RegionSize)
                return true;
        }
        return false;
    }
private:
    // Region start height
    int RegionStart = -15;
    int RegionEnd = -310;

    // Portion of the -1 to 1 value that this region occupies
    float RegionSize = -0.9f;
};