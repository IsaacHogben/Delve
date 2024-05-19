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

UENUM(BlueprintType)
enum class EGenerationLayer : uint8
{
	TerrainLayer, InterChunkLayer, Complete
};

UENUM(BlueprintType)
enum class EDeliverUpdateDirection : uint8
{
	None, North, South, East, West, Up, Down
};