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
        Subsoil = EBlock::CliffStone;
        Bedrock = EBlock::CliffStone;

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
        // Specific implementation for Base region
        return EBlock::Null;
    }
    bool IsInRegion(float& x, float& y, float& z, float & Value, float & UpValue) const
    {
        // Specific implementation for Cliff region
        if ((z > RegionEnd && z < RegionStart) || (z > 12 && z < 200))
        {
            //Attempt to exclude parts from being in Cliif Region based on their slope
            float slope = UpValue - Value;
            if (slope < 0.008f)
            {
                if (Noise->GetNoise(x, y, z) < RegionSize)
                    return true;
            }
        }
        return false;
    }
private:
    // Region start height
    int RegionStart = -18;
    int RegionEnd = -370;

    // Portion of the -1 to 1 value that this region occupies
    float RegionSize = -0.9f;
};