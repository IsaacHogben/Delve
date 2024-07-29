// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Utils/FastNoiseLite.h"

#include "NoiseManager.generated.h"

FastNoiseLite;

UENUM(BlueprintType)
enum class ENoiseType : uint8
{
	NoiseType_OpenSimplex2,
	NoiseType_OpenSimplex2S,
	NoiseType_Cellular,
	NoiseType_Perlin,
	NoiseType_ValueCubic,
	NoiseType_Value
};

UENUM(BlueprintType)
enum class EFractalType : uint8
{
	FractalType_None,
	FractalType_FBm,
	FractalType_Ridged,
	FractalType_PingPong,
	FractalType_DomainWarpProgressive,
	FractalType_DomainWarpIndependent
};

UENUM(BlueprintType)
enum class ECellularDistanceFunction : uint8
{
	CellularDistanceFunction_Euclidean,
	CellularDistanceFunction_EuclideanSq,
	CellularDistanceFunction_Manhattan,
	CellularDistanceFunction_Hybrid
};

UENUM(BlueprintType)
enum class ECellularReturnType : uint8
{
	CellularReturnType_CellValue,
	CellularReturnType_Distance,
	CellularReturnType_Distance2,
	CellularReturnType_Distance2Add,
	CellularReturnType_Distance2Sub,
	CellularReturnType_Distance2Mul,
	CellularReturnType_Distance2Div
};

UENUM(BlueprintType)
enum class EDomainWarpType : uint8
{
	DomainWarpType_OpenSimplex2,
	DomainWarpType_OpenSimplex2Reduced,
	DomainWarpType_BasicGrid,
	DomainWarpType_None
};

USTRUCT(BlueprintType)
struct FFastNoise
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "FastNoise")
	FString NoiseName;
	FastNoiseLite* Noise;

	UPROPERTY(EditAnywhere, Category = "FastNoise")
	float Frequency = 0.01f;
	UPROPERTY(EditAnywhere, Category = "FastNoise")
	ENoiseType NoiseType = ENoiseType::NoiseType_Value;

	// Fractal Settings
	UPROPERTY(EditAnywhere, Category = "FastNoise | Fractal")
	EFractalType FractalType = EFractalType::FractalType_None;

	UPROPERTY(EditAnywhere, Category = "FastNoise", meta = (EditCondition = "FractalType != EFractalType::FractalType_None", EditConditionHides))
	int FractalOctaves = 3;

	UPROPERTY(EditAnywhere, Category = "FastNoise", meta = (EditCondition = "FractalType != EFractalType::FractalType_None", EditConditionHides))
	float FractalLunaricity = 2;

	UPROPERTY(EditAnywhere, Category = "FastNoise", meta = (EditCondition = "FractalType != EFractalType::FractalType_None", EditConditionHides))
	float FractalGain = 0.5f;

	// Cellular Noise Settings
	UPROPERTY(EditAnywhere, Category = "FastNoise", meta = (EditCondition = "NoiseType == ENoiseType::NoiseType_Cellular", EditConditionHides))
	ECellularDistanceFunction CellularDistanceFunction = ECellularDistanceFunction::CellularDistanceFunction_Euclidean;

	UPROPERTY(EditAnywhere, Category = "FastNoise", meta = (EditCondition = "NoiseType == ENoiseType::NoiseType_Cellular", EditConditionHides))
	ECellularReturnType CellularReturnType = ECellularReturnType::CellularReturnType_Distance;

	UPROPERTY(EditAnywhere, Category = "FastNoise", meta = (EditCondition = "NoiseType == ENoiseType::NoiseType_Cellular", EditConditionHides))
	float CellularJitter = 1;

	// Domain Warp Settings
	UPROPERTY(EditAnywhere, Category = "FastNoise")
	EDomainWarpType DomainWarpType = EDomainWarpType::DomainWarpType_None;

	UPROPERTY(EditAnywhere, Category = "FastNoise", meta = (EditCondition = "DomainWarpType != EDomainWarpType::DomainWarpType_None", EditConditionHides))
	float DomainWarpAmplitude = 1;

	UPROPERTY(EditAnywhere, Category = "FastNoise", meta = (EditCondition = "DomainWarpType != EDomainWarpType::DomainWarpType_None", EditConditionHides))
	float DomainWarpFrequency = 0.1;
};

UCLASS()
class UNoiseManager : public UObject
{
	GENERATED_BODY()

public:
	~UNoiseManager();

	void InitializeArray(TArray<FFastNoise> NoiseArray);
	FastNoiseLite* InitializeNoise(FFastNoise& NoiseSettings);

	// Noise for level 1
	FastNoiseLite* InputNoise;
	FastNoiseLite* WorldHeightNoise;
	FastNoiseLite* WorldHeightCellDensityNoise;
};
