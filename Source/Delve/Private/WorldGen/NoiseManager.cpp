// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGen/NoiseManager.h"

UNoiseManager::~UNoiseManager()
{
	UE_LOG(LogTemp, Warning, TEXT("NoiseManager Deconstructor called"));
}

void UNoiseManager::InitializeArray(TArray<FFastNoise> NoiseArray)
{
	UE_LOG(LogTemp, Warning, TEXT("NoiseArray.Num() == %d"), NoiseArray.Num());
	if (NoiseArray.Num() == 2)
	{
		BaseNoise = InitializeNoise(NoiseArray[0]);
		WorldHeightCellNoise = InitializeNoise(NoiseArray[1]);
		WorldHeightCellDensityNoise = InitializeNoise(NoiseArray[1]);
		WorldHeightCellDensityNoise->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
	}
}

FastNoiseLite* UNoiseManager::InitializeNoise(FFastNoise& NoiseSettings)
{
	FastNoiseLite* Noise = new FastNoiseLite(999);
	Noise->SetFrequency(NoiseSettings.Frequency);
	Noise->SetNoiseType(static_cast<FastNoiseLite::NoiseType>(NoiseSettings.NoiseType));

	Noise->SetFractalType(static_cast<FastNoiseLite::FractalType>(NoiseSettings.FractalType));
	Noise->SetFractalOctaves(NoiseSettings.FractalOctaves);
	Noise->SetFractalLacunarity(NoiseSettings.FractalGain);
	Noise->SetFractalGain(NoiseSettings.FractalLunaricity);

	Noise->SetDomainWarpType(static_cast<FastNoiseLite::DomainWarpType>(NoiseSettings.DomainWarpType));
	Noise->SetDomainWarpAmp(NoiseSettings.DomainWarpAmplitude);

	Noise->SetCellularDistanceFunction(static_cast<FastNoiseLite::CellularDistanceFunction>(NoiseSettings.CellularDistanceFunction));
	Noise->SetCellularReturnType(static_cast<FastNoiseLite::CellularReturnType>(NoiseSettings.CellularReturnType));
	Noise->SetCellularJitter(NoiseSettings.CellularJitter);

	return Noise;
}