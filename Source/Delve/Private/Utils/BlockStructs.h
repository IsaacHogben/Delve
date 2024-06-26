// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Enums.h"

#include "BlockStructs.generated.h"

USTRUCT(BlueprintType)
struct FBlockData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintType)
	EBlock Block;
	UPROPERTY(EditDefaultsOnly, BlueprintType)
	bool GreedyMesh = true;
	UPROPERTY(EditDefaultsOnly, BlueprintType)
	EBlockDisplayType DisplayFaces;
	UPROPERTY(EditDefaultsOnly, BlueprintType)
	bool IsTwoSided = false;
	UPROPERTY(EditDefaultsOnly, BlueprintType)
	bool UseBlockWorldPositionColor = true;
	UPROPERTY(EditDefaultsOnly, BlueprintType)
	uint8 Face_WPO;
	UPROPERTY(EditDefaultsOnly, BlueprintType)
	int OpacityMask;
	
};