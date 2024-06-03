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
    FastNoiseLite* CliffNoise;

    UCliffRegion()
    {
        Topsoil = EBlock::Grass;
        Subsoil = EBlock::Stone;
        Bedrock = EBlock::Stone;

        CliffNoise = new FastNoiseLite();
        CliffNoise->SetFrequency(0.008);
        CliffNoise->SetNoiseType(FastNoiseLite::NoiseType_Cellular);

        CliffNoise->SetFractalType(FastNoiseLite::FractalType_None);
        CliffNoise->SetFractalOctaves(0);
        CliffNoise->SetFractalLacunarity(0);
        CliffNoise->SetFractalGain(0);

        CliffNoise->SetDomainWarpType(FastNoiseLite::DomainWarpType_BasicGrid);
        CliffNoise->SetDomainWarpAmp(0);

        CliffNoise->SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
        CliffNoise->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Sub);
        CliffNoise->SetCellularJitter(1);
    }

    virtual EBlock GetBlock(ESoilLayer SoilLayer) const override
    {
        // Specific implementation for Base region
        return EBlock::Null;
    }
    virtual bool IsInRegion(float& x, float& y, float& z) const override
    {
        // Specific implementation for Base region
        if (z > -15)
            return false;
        if (CliffNoise->GetNoise(x, y, z) < -0.9f)
            return true;
        return false;
    }
};
