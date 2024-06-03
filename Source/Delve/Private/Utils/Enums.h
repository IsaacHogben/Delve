#pragma once

UENUM(BlueprintType)
enum class EDirection : uint8
{
	North, East, South, West, Up, Down
};

UENUM(BlueprintType)
enum class EBlock : uint8
{
	Null, Air, Stone, Dirt, Grass, Leaves
};

UENUM(BlueprintType)
enum class ELocalRegion : uint8
{
	Base, Cliffs
};

UENUM(BlueprintType)
enum class ESoilLayer : uint8
{
	Topsoil, Subsoil, Bedrock
};

UENUM(BlueprintType)
enum class ECompletedGenerationLayer : uint8
{
	Empty,
	//Terrain and local decorations generated in this layer. Any decorations that fall outside chunk bounds are distributed.
	InitialTerrainLayer,
	//Terrain generation complete, this layer is eligible to be decorated
	HasNeigboursLayer,
	//Decorations received from other chunks are processed in this layer.
	DecorationLayer,
	//Set to Complete once chunk is fully generated and Mesh is added to world.
	Complete,
	//Used by completed chunks that are chached or saved
	CompleteInActive
};
