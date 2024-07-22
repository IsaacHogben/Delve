// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldGen/GenClasses/LocalRegion.h"
#include "DomeRegion.generated.h"

/**
 * 
 */
UCLASS()
class UDomeRegion : public ULocalRegion
{
	GENERATED_BODY()
	
public:
	UDomeRegion()
	{
		Topsoil = EBlock::Dirt;
		Subsoil = EBlock::Dirt;
		Bedrock = EBlock::CliffStone;
	}
};
