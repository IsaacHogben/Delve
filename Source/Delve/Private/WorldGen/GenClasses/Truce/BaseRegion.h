// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldGen/GenClasses/LocalRegion.h"
//#include "Utils/Enums.h"

#include "BaseRegion.generated.h"

/**
 * 
 */
UCLASS()
class UBaseRegion : public ULocalRegion
{
	GENERATED_BODY()
	
public:
    UBaseRegion()
    {
    };

    virtual EBlock GetBlock(ESoilLayer SoilLayer) const override
    {
        // Specific implementation for Base region

        return EBlock::Null;
    }
    virtual bool IsInRegion(float& x, float& y, float& z) const override
    {
        // Specific implementation for Base region

        return true;
    }
};
