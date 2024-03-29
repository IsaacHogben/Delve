#pragma once

UENUM(BlueprintType)
enum class EDirection : uint8
{
	North, East, South, West, Up, Down
};

UENUM(BlueprintType)
enum class EBlock : uint8
{
	Null, Air, Stone, Dirt, Grass
};
