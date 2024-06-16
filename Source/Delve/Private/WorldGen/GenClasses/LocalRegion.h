// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../../Utils/Enums.h"

#include "LocalRegion.generated.h"
/**
 * 
 */
UCLASS()
class ULocalRegion : public UObject
{
	GENERATED_BODY()

public:
	int Seed = 3353;
	EBlock Topsoil;
	EBlock Subsoil;
	EBlock Bedrock;

	// Default constructor
	ULocalRegion()
		: Topsoil(EBlock::Grass)
		, Subsoil(EBlock::Dirt)
		, Bedrock(EBlock::Stone)
	{
	}
	ULocalRegion(EBlock topsoil, EBlock subsoil, EBlock bedrock)
		: Topsoil(topsoil)
		, Subsoil(subsoil)
		, Bedrock(bedrock)
	{
	}
    virtual EBlock GetBlock(ESoilLayer SoilLayer) const
    {
        // Default implementation (if any)
        return EBlock::Null;
    }
};
