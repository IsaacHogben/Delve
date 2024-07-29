#pragma once

// Block Enums
UENUM(BlueprintType)
enum class EBlock : uint8
{
	Null, Air, Stone, Dirt, Grass, Leaves, CliffStone, Moss, Log, Vine, Ruin, SurfaceGrass, WhiteStone
};

UENUM(BlueprintType)
enum class EBlockDisplayType : uint8
{
	All, OnlySides
};

UENUM(BlueprintType)
enum class ELocalRegion : uint8
{
	Base, Cliffs
};

UENUM(BlueprintType)
enum class ESoilLayer : uint8
{
	Topsoil, Subsoil, Bedrock, SubsoilOverride, Override
};

UENUM(BlueprintType)
enum class EFoliageType : uint8
{
	Sphere
};

UENUM(BlueprintType)
enum class ECompletedGenerationLayer : uint8
{
	Empty,
	//Terrain and local decorations generated in this layer. Any decorations that fall outside chunk bounds are distributed.
	InitialTerrainLayer,
	//Terrain generation complete, this layer is eligible to be decorated
	GenerateDecorationLayer,
	//Decorations received from other chunks are processed in this layer.
	CompleteDecorationLayer,
	//Set to Complete once chunk is fully generated and Mesh is added to world.
	Complete,
	//Used by completed chunks that are chached or saved
	CompleteInActive
};

UENUM(BlueprintType)
enum class EMeshType : uint8
{
	OpaqueCollision
};