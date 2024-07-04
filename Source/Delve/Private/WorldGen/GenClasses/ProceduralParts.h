// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralParts.generated.h"

/**
 * 
 */
 // L-system structure
USTRUCT(BlueprintType)
struct FTreeSystem : public FTableRowBase 
{

		GENERATED_BODY();

	public:
		UPROPERTY(EditAnywhere)
		FString Axiom;

		TMap<TCHAR, FString> Rules;
		UPROPERTY(EditAnywhere)
		int Iterations;
		UPROPERTY(EditAnywhere)
		float Angle = 45;
		UPROPERTY(EditAnywhere)
		float AngleDeviation = 15;
		UPROPERTY(EditAnywhere)
		float TrunkDeviation = 25;
		UPROPERTY(EditAnywhere)
		int BaseBranchLength = 5;
		UPROPERTY(EditAnywhere)
		int MinBranchLength = 3;
		UPROPERTY(EditAnywhere)
		int BaseTrunkWidth;
		UPROPERTY(EditAnywhere)
		int MinBranchWidth;
		UPROPERTY(EditAnywhere)
		int LeafSize;
		UPROPERTY(EditAnywhere)
		EFoliageType FoliageType;
		UPROPERTY(EditAnywhere)
		bool HasVines;
		UPROPERTY(EditAnywhere)
		EBlock LeafBlock;
		UPROPERTY(EditAnywhere)
		EBlock WoodBlock;
		UPROPERTY(EditAnywhere)
		TArray<EBlock> SpawnsOn;
		UPROPERTY(EditAnywhere)
		int SpawnRate;
		UPROPERTY(EditAnywhere)
		FIntVector SpawnPosOffset;
};

// Tree standard
// TF-[&F][^F]++[/F][\F]-[&F-[&F][^F]++[/F][\F]][^F-[&F][^F]++[/F][\F]]++[/F-[&F][^F]++[/F][\F]][\F-[&F][^F]++[/F][\F]]

// Tree Wide used above
// 'F', "F-[&F][^F]++[/F][\\F]"
//s
//F-[&F][^F]++[/F][\F]-[&F-[&F][^F]++[/F][\F]][^F-[&F][^F]++[/F][\F]]++[/F-[&F][^F]++[/F][\F]][\F-[&F][^F]++[/F][\F]]
//m
//TF-[&F][^F]++[/F][\F]-[&F-[&F][^F]++[/F][\F]][^F-[&F][^F]++[/F][\F]]++[/F-[&F][^F]++[/F][\F]][\F-[&F][^F]++[/F][\F]]
//l
//TTTF-[&F][^F]++[/F][\F]-[&F-[&F][^F]++[/F][\F]][^F-[&F][^F]++[/F][\F]]++[/F-[&F][^F]++[/F][\F]][\F-[&F][^F]++[/F][\F]]-[&F-[&F][^F]++[/F][\F]-[&F-[&F][^F]++[/F][\F]][^F-[&F][^F]++[/F][\F]]++[/F-[&F][^F]++[/F][\F]][\F-[&F][^F]++[/F][\F]]][^F-[&F][^F]++[/F][\F]-[&F-[&F][^F]++[/F][\F]][^F-[&F][^F]++[/F][\F]]++[/F-[&F][^F]++[/F][\F]][\F-[&F][^F]++[/F][\F]]]++[/F-[&F][^F]++[/F][\F]-[&F-[&F][^F]++[/F][\F]][^F-[&F][^F]++[/F][\F]]++[/F-[&F][^F]++[/F][\F]][\F-[&F][^F]++[/F][\F]]][\F-[&F][^F]++[/F][\F]-[&F-[&F][^F]++[/F][\F]][^F-[&F][^F]++[/F][\F]]++[/F-[&F][^F]++[/F][\F]][\F-[&F][^F]++[/F][\F]]]