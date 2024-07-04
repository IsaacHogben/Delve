// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGen/ProceduralTerrain.h"
#include "NoiseManager.h"

AProceduralTerrain::AProceduralTerrain()
{
	UE_LOG(LogTemp, Warning, TEXT("ProceduralTerrain Constructor Called"));
	//PrimaryActorTick.bCanEverTick = true;
}

AProceduralTerrain::~AProceduralTerrain()
{
	UE_LOG(LogTemp, Warning, TEXT("ProceduralTerrain Deconstructor called"));
}

// Called when the game starts or when spawned
void AProceduralTerrain::Initialize()
{
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay called in ProceduralTerrain"));
	N = NewObject<UNoiseManager>();
	N->InitializeArray(GenerationNoiseArray);
	// Initialize regions
	BaseRegion = NewObject<UBaseRegion>();
	CliffRegion = NewObject<UCliffRegion>();
	// Load TreeData into memory
	TreeDataTable->GetAllRows("", TreeDataArray);
	//IMGrass->
	//InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	HISMC->SetStaticMesh(IMGrass);
	HISMC->SetMaterial(0, GrassMat);
	//HISMC->Material
	//HISMC->Material

	//for (int i = 0; i < 20; i++)
	//{
	//	for (int j = 0; j < 50; j++)
	//	{
	//		for (int k = 0; k < 1000; k++)
	//		{
	//			FTransform InstanceTransform;
	//			InstanceTransform.SetLocation(FVector(i * 200, j * 200, k * 200)); // Adjust location as needed
	//			HISMC->AddInstance(InstanceTransform);

	//		}
	//	}
	//}
}

int QuantizeCoordinate(int value, int quantizationStep)
{
	if (quantizationStep == 0)
		return value;
	return (value / quantizationStep) * quantizationStep;
}

float GetQuantizedNoise(int x, int y, int z, FastNoiseLite* Noise)
{

	int QuantizationStep = 3;
	// Quantize the input coordinates
	float quantizedX = QuantizeCoordinate(x, QuantizationStep);
	float quantizedY = QuantizeCoordinate(y, QuantizationStep);
	float quantizedZ = QuantizeCoordinate(z, QuantizationStep);

	// Get the noise value using quantized coordinates
	return Noise->GetNoise(float(x), float(y), quantizedZ);
}

int AProceduralTerrain::GetBlockIndex(int X, int Y, int Z)
{
	int IndexSize = 262144;
	int r = Z * ChunkSize * ChunkSize + Y * ChunkSize + X;
	if (r >= IndexSize)
		return 0; //get outside chunk!
	return r;
}

EBlock AProceduralTerrain::GetBlockFromRegion(ULocalRegion* LocalRegion, ESoilLayer SoilLayer)
{
	switch (SoilLayer) {
	case ESoilLayer::Topsoil:
		return LocalRegion->Topsoil;
	case ESoilLayer::Subsoil:
		return LocalRegion->Subsoil;
	case ESoilLayer::Bedrock:
		return LocalRegion->Bedrock;
	}

	return EBlock::Null;
}

EBlock AProceduralTerrain::GetTerrainLevelOne(float x, float y, float z, EBlock AboveBlock)
{
	ESoilLayer SoilLayer = ESoilLayer::Bedrock;
	ULocalRegion* LocalRegion = BaseRegion;
	float ZSquish = 1.2;
	float Density = ZDensityCurve->GetFloatValue(z);
	float UpDensity = ZDensityCurve->GetFloatValue(z + 1);
	auto Value = BaseRegion->Noise->GetNoise(x, y, z * ZSquish);
	auto UpValue = BaseRegion->Noise->GetNoise(x, y, (z+1) * ZSquish);
	float UpZ = z + 1;
	const int CliffZMod = 2;

	// Alter value based on Cliff Region
	if (CliffRegion->IsInRegion(x, y, z, Value, UpValue))
	{
		z = QuantizeCoordinate(z, 6);
		LocalRegion = CliffRegion;
		Value = BaseRegion->Noise->GetNoise(x, y, (z - CliffZMod) * ZSquish);
		//UpValue = N->BaseNoise->GetNoise(x, y, (z + 1) * ZSquish);
	}	
	if (CliffRegion->IsInRegion(x, y, UpZ, Value, UpValue))
	{
		UpZ = QuantizeCoordinate(z, 6);
		LocalRegion = CliffRegion;
		UpValue = BaseRegion->Noise->GetNoise(x, y, (UpZ - CliffZMod) * ZSquish);
	}

	//Alter Density for World Edge
	int WorldRadius = 768;
	float Distance = FMath::Sqrt(FMath::Square(x - WorldCenter.X) + FMath::Square(y - WorldCenter.Y));
	if (Distance >= WorldRadius)
	{
		float WorldEdgeDropoff = WorldEdgeDensityCurve->GetFloatValue(Distance - WorldRadius);
		Density += WorldEdgeDropoff;
		UpDensity += WorldEdgeDropoff;
		if (Density >= 1)
			return EBlock::CliffStone;
	}
	else
	{
		//Alter Density for World Surface
		int MaxWorldheight = 25;
		int Offset = MaxWorldheight / 2;
		float WorldHeight = N->WorldHeightNoise->GetNoise(x, y);

		WorldHeight = WorldHeight * MaxWorldheight;
		if (z > WorldHeight + Offset && Value < 0)
		{
			Density = -1;
		}
		if (z + 1 > WorldHeight + Offset && Value < 0)
		{
			UpDensity = -1;
		}
	}

	// Pillar calculations to move or delete vvvv
	/*if (z >= 0)
	{	
		const int TransitionHeight = 6;
		const int MaxPillarHeight = 32;
		const int MinPillarHeight = 0;
		const float PillarDensity = 0.999;
		const float WorldCellHeight = FMath::Clamp(FMath::RoundToInt(N->WorldHeightCellNoise->GetNoise(x, y) * 100), TransitionHeight + MinPillarHeight, MaxPillarHeight);

		float WorldHieghtCellDensity = N->WorldHeightCellDensityNoise->GetNoise(x, y) * -1;// Invert
		WorldHieghtCellDensity -= PillarDensity;

		if (z < TransitionHeight)
		{
			if (z != 0)
				Density = FMath::Lerp(0, WorldHieghtCellDensity, z / TransitionHeight);
			UpDensity = FMath::Lerp(0, WorldHieghtCellDensity, (z + 1) / TransitionHeight);
		}
		else if (z < WorldCellHeight)
		{
			if (z != 0)
				Density = WorldHieghtCellDensity;
			UpDensity = WorldHieghtCellDensity;
		}
		else if (z >= WorldCellHeight)
		{
			//if (z != WorldCellHeight)
				Density = FMath::Lerp(WorldHieghtCellDensity, -2, (z - WorldCellHeight) / TransitionHeight);
			UpDensity = FMath::Lerp(WorldHieghtCellDensity, -2, (z + 1 - WorldCellHeight) / TransitionHeight);
		}
	}*/
	

	if (IsAir(Value, Density))
		return EBlock::Air;
	if (LocalRegion == CliffRegion)
	{
		if (AboveBlock == EBlock::Air)
			return GetBlockFromRegion(CliffRegion, ESoilLayer::Topsoil);
		else if (AboveBlock != EBlock::Null)
			return GetBlockFromRegion(CliffRegion, ESoilLayer::Bedrock);
	}
	else if (IsAir(UpValue, UpDensity))
		SoilLayer = ESoilLayer::Topsoil;
	else if (UpValue > Value) //In Top half of terrain
	{
		if (Value >= (Density - 0.022f))
			SoilLayer = ESoilLayer::Subsoil;
	}

	return GetBlockFromRegion(LocalRegion, SoilLayer);
}


//Generates first layer terrain and returns FBulkBlockUpdate for additional levels of modification.
TArray<FCachedBlockUpdate> AProceduralTerrain::GetGeneratedChunk(FVector ChunkPosition, FIntVector ChunkVectorPosition, TArray<EBlock>& BlockArray, bool& IsChunkEmpty)
{
	TArray<FCachedBlockUpdate> BlockUpdates;

	IsChunkEmpty = false; //WILL NEVER TRIGER! TODO
	EBlock Block;
	EBlock AboveBlock = EBlock::Null;
	for (int x = 0; x < ChunkSize; ++x)
	{
		for (int y = 0; y < ChunkSize; ++y)
		{
			for (int z = ChunkSize - 1; z >= 0 ; --z)
			{				
				if (0)// TEST NOISE - Used to test new noise values on their own
				{
					float noise = N->BaseNoise->GetNoise(float(x) + ChunkPosition.X, float(y) + ChunkPosition.Y, float(z) + ChunkPosition.Z);
					if (noise > ZDensityCurve->GetFloatValue(z + ChunkPosition.Z))
						BlockArray[GetBlockIndex(x, y, z)] = EBlock::Air;
					else
						BlockArray[GetBlockIndex(x, y, z)] = EBlock::CliffStone;
				}
				else// normal
				{
					if (AboveBlock == EBlock::Null)
						AboveBlock = GetTerrainLevelOne(x + ChunkPosition.X, y + ChunkPosition.Y, z + ChunkPosition.Z + 1, EBlock::Null);
					Block = GetTerrainLevelOne(x + ChunkPosition.X, y + ChunkPosition.Y, z + ChunkPosition.Z, AboveBlock);
					
					BlockArray[GetBlockIndex(x, y, z)] = Block;

					if (Block != EBlock::Air)
						IsChunkEmpty = false;

					//Reset for next loop
					AboveBlock = Block;
				}
			}
		}
	}

	//AddReferencelessDecorations(BlockArray, Noise, BlockUpdates);
	//MakeTestShape(BlockUpdates, -1,-1,-1);
	//MakeTestShape(BlockUpdates, 0,0,0);
	return BlockUpdates;
}

bool AProceduralTerrain::IsSurfaceBlock(float UpValue, float Density)
{
	if (IsAir(UpValue, Density))
		return true;
	return false;
}

bool AProceduralTerrain::IsAir(float Value, float Density)
{
	if (Value >= Density)
		return true;
	return false;
}

void AProceduralTerrain::AddReferencelessDecorations(TArray<EBlock>& BlockArray, FastNoiseLite* Noise, TArray<FCachedBlockUpdate>& BlockUpdates)
{
	EBlock Block;
	for (int x = 0; x < ChunkSize + 2; ++x)
	{
		for (int y = 0; y < ChunkSize + 2; ++y)
		{
			for (int z = 0; z < ChunkSize + 2; ++z)
			{
				 Block = BlockArray[GetBlockIndex(x, y, z)];
				//check for condition and make changes
				if (Block == EBlock::Grass && BlockArray[GetBlockIndex(x, y, z + 1)] == EBlock::Air)
				{
					if (FMath::RandRange(0, 124) == 0)
					{
						MakeTestShape(BlockUpdates, x + 1, y, z + 1);
						MakeTestShape(BlockUpdates, x + 1, y + 1, z + 1);
						MakeTestShape(BlockUpdates, x, y+ 1, z + 1);
						MakeTestShape(BlockUpdates, x, y, z + 1);
					}
				}
			}
		}
	}
}

TArray<FCachedBlockUpdate> AProceduralTerrain::AddDecorationsWithContext(TArray<EBlock>& BlockArray, UChunkClass* Chunk)
{
	TArray<FCachedBlockUpdate> BlockUpdates;
	for (int x = 0; x < ChunkSize; ++x)
	{
		for (int y = 0; y < ChunkSize; ++y)
		{
			for (int z = ChunkSize - 1; z >= 0; --z)
			{
				EBlock Block = Chunk->GetBlock(FIntVector(x, y, z), true);
				if (Block != EBlock::Null)
				{
					// TREES
					for (const auto Tree : TreeDataArray)
					{
						if ((Tree->SpawnsOn.Contains(Block) && FMath::RandRange(0, Tree->SpawnRate) == 0))
						{
							GenerateTree(BlockUpdates, Chunk, x + Tree->SpawnPosOffset.X, y + Tree->SpawnPosOffset.Y, z + Tree->SpawnPosOffset.Z, *Tree);
							break;
						}
					}
					// VINES
					if (Block == EBlock::Air)
					{
						//Some Vines on Stone
						if (FMath::RandRange(0, 400) == 0 && Chunk->GetBlock(FIntVector(x, y, z + 1), true) == EBlock::Stone)
							MakeTestVine(BlockUpdates, x, y, z);
						//More Vines on Dirt
						else if (FMath::RandRange(0, 12) == 0 && Chunk->GetBlock(FIntVector(x, y, z + 1), true) == EBlock::Dirt)
							MakeTestVine(BlockUpdates, x, y, z);
					}
					if (Block == EBlock::Grass && FMath::RandRange(0, 15) == 0)
					{
						//FTransform InstanceTransform;
						//InstanceTransform.SetLocation(Chunk->ChunkWorldPosition + (FVector(x, y, z + 1) * 50)); // Adjust location as needed
						InstancedMeshPositions.Add((Chunk->ChunkWorldPosition + FVector(x, y, z)) * 50);
						//HISMC->AddInstance(InstanceTransform);
					}
				}
			}
		}
	}
	if (Chunk->ChunkData->Position == FIntVector(0,0,-6))
	{
		GenerateTestTreeAtLocation(BlockUpdates, Chunk, 32, 32, -6);
	}
	AsyncTask(ENamedThreads::GameThread, [this]()
		{
			for (const auto& pos : InstancedMeshPositions)
			{
				FTransform InstanceTransform;
				//FQuat InstanceRotation = FQuat::MakeFromEuler(FVector(0, 0, FMath::RandRange(-90, 90)));//
				//InstanceTransform.SetRotation(InstanceRotation);
				InstanceTransform.SetLocation(pos);
				HISMC->AddInstance(InstanceTransform);
			}
			InstancedMeshPositions.Empty();
		});
	return BlockUpdates;
}

void AProceduralTerrain::MakeTestShape(TArray<FCachedBlockUpdate>& BlockUpdates, int x, int y, int z)
{
	//BlockUpdates.Add(FBlockUpdate(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector(x, y, z), EBlock::Stone));
	//int r = 44;
	for (int i = 1; i < 64; i++)
	{ 	
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x + i, y, z), EBlock::Stone));
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x, y + i, z), EBlock::Stone));
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x, y, z + i), EBlock::Stone));
	}
	x = 63;
	y = 63;
	z = 63;
	for (int i = 1; i < 64; i++)
	{
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x - i, y, z), EBlock::Stone));
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x, y - i, z), EBlock::Stone));
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x, y, z - i), EBlock::Stone));
	}
}

void AProceduralTerrain::MakeTestVine(TArray<FCachedBlockUpdate>& BlockUpdates, int x, int y, int z)
{
	int length = FMath::RandRange(1, 24);

	for (int i = 0; i < length; i++)
	{
		BlockUpdates.Add(FCachedBlockUpdate(FIntVector(x, y, z - i), EBlock::Vine));
	}
}

void AProceduralTerrain::AddCylinder(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int height, int centerX, int centerY, int baseZ, EBlock blockType)
{
	for (int z = 0; z < height; ++z)
	{
		for (int x = -radius; x <= radius; ++x)
		{
			for (int y = -radius; y <= radius; ++y)
			{
				if (x * x + y * y <= radius * radius)
				{
					BlockUpdates.Add(FCachedBlockUpdate( FIntVector(centerX + x, centerY + y, baseZ + z), blockType));
				}
			}
		}
	}
}

void AProceduralTerrain::AddSphere(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int centerX, int centerY, int centerZ, EBlock blockType)
{
	for (int x = -radius; x <= radius; ++x)
	{
		for (int y = -radius; y <= radius; ++y)
		{
			for (int z = -radius; z <= radius; ++z)
			{
				if (x * x + y * y + z * z <= radius * radius)
				{
					BlockUpdates.Add(FCachedBlockUpdate(FIntVector(centerX + x, centerY + y, centerZ + z), blockType));
				}
			}
		}
	}
}

void AProceduralTerrain::AddCanopy(TArray<FCachedBlockUpdate>& BlockUpdates, int radius, int centerX, int centerY, int centerZ, EBlock blockType, uint8 density)
{
	for (int x = -radius; x <= radius; ++x)
	{
		for (int y = -radius; y <= radius; ++y)
		{
			for (int z = -radius; z <= radius; ++z)
			{
				if (FMath::RandRange(0, 255) <= density && x * x + y * y + z * z <= radius * radius)
				{
					BlockUpdates.Add(FCachedBlockUpdate(FIntVector(centerX + x, centerY + y, centerZ + z), blockType));
				}
			}
		}
	}
}

// Function to apply L-system rules
FString ApplyRules(const FTreeSystem& system, int iterations) {
	FString currentString = system.Axiom;
	for (int i = 0; i < iterations; ++i) {
		FString nextString;
		for (TCHAR symbol : currentString) {
			if (system.Rules.Contains(symbol)) {
				nextString += system.Rules[symbol];
			}
			else {
				nextString += symbol;
			}
		}
		currentString = nextString;
		UE_LOG(LogTemp, Warning, TEXT("R = %s"), *currentString);
	}
	return currentString;
}

// Function to generate a tree with the specified parameters
void AProceduralTerrain::GenerateTree(TArray<FCachedBlockUpdate>& BlockUpdates, UChunkClass* Chunk, int x, int y, int z, const FTreeSystem& system) {
	FString treeString = ApplyRules(system, system.Iterations);

	// Initialize position, direction, and branch length
	FVector position(x, y, z);
	FRotator direction(0, 0, 0);

	// Stack to save and restore positions and directions
	std::stack<FVector> positionStack;
	std::stack<FRotator> directionStack;
	std::stack<int> widthStack;
	std::stack<int> branchLengthStack;

	// Initialize random stream for randomness
	FRandomStream randomStream;
	randomStream.GenerateNewSeed();

	int currentBranchLength = system.BaseBranchLength;
	int currentWidth = system.BaseTrunkWidth;

	// Apply Random rotation to the tree.
	direction.Yaw += randomStream.FRandRange(-90, 90);

	for (TCHAR symbol : treeString) {
		// Reinterpret trunk
		if (symbol == 'T') {
			direction.Yaw += randomStream.FRandRange(-system.TrunkDeviation, system.TrunkDeviation);
			direction.Pitch += randomStream.FRandRange(-system.TrunkDeviation, system.TrunkDeviation);
			direction.Roll += randomStream.FRandRange(-system.TrunkDeviation, system.TrunkDeviation);
			symbol = 'F';
		}
		else if (symbol == 'A' || symbol == 'B')
			symbol = 'F';

		switch (symbol) {
		case 'F': {
			// Move forward by branch length and place log blocks
			for (int i = 0; i < currentBranchLength; ++i) {
				position += direction.RotateVector(FVector(0, 0, 1));  // Ensure we move in the Z direction for vertical growth

				// Check if the new position is valid
				//if (Chunk->GetBlock(FIntVector(position.X, position.Y, position.Z), true) == EBlock::Air) {
					// Place log blocks according to the width as a cylinder
					float radius = currentWidth / 2;  // Calculate radius
					float centerOffset = (currentWidth + 1) % 2;

					for (float dx = -radius + centerOffset; dx <= radius; ++dx) {
						for (float dy = -radius + centerOffset; dy <= radius; ++dy) {
							for (float dz = -radius + centerOffset; dz <= radius; ++dz) {
								FVector logPos = position + FVector(dx, dy, dz);
								// Adjust the distance calculation for voxel representation
								if (FVector::Dist2D(FVector(0, 0, 0), FVector(dx, dy, 0)) <= radius) {
									BlockUpdates.Add(FCachedBlockUpdate(FIntVector(logPos.X, logPos.Y, logPos.Z), system.WoodBlock));
								}
							}
						}
					}
				//}
				//else {
					//return;// break; // Stop if space is not empty
				//}
			}
			// Reduce width gradually for branches
			if (currentWidth > 1) {
				currentWidth = FMath::Max(system.MinBranchWidth, currentWidth - 1);
			}
			// Reduce branch length gradually for branches
			currentBranchLength = FMath::Max(system.MinBranchLength, currentBranchLength - randomStream.RandRange(0, 1));
			break;
		}
		case '+': {
			// Turn right around the Z-axis with random deviation
			direction.Yaw += system.Angle + randomStream.FRandRange(-system.AngleDeviation, system.AngleDeviation);
			break;
		}
		case '-': {
			// Turn left around the Z-axis with random deviation
			direction.Yaw -= system.Angle + randomStream.FRandRange(-system.AngleDeviation, system.AngleDeviation);
			break;
		}
		case '&': {
			// Pitch down (rotate around the Y-axis) with random deviation, limit droopiness
			float newPitch = direction.Pitch + system.Angle + randomStream.FRandRange(-system.AngleDeviation, system.AngleDeviation);
			/*if (newPitch < 90) {
				direction.Pitch = newPitch;
			}*/
			break;
		}
		case '^': {
			// Pitch up (rotate around the Y-axis) with random deviation
			direction.Pitch -= system.Angle + randomStream.FRandRange(-system.AngleDeviation, system.AngleDeviation);
			break;
		}
		case '\\': {
			// Roll left (rotate around the X-axis) with random deviation
			direction.Roll += system.Angle + randomStream.FRandRange(-system.AngleDeviation, system.AngleDeviation);
			break;
		}
		case '/': {
			// Roll right (rotate around the X-axis) with random deviation
			direction.Roll -= system.Angle + randomStream.FRandRange(-system.AngleDeviation, system.AngleDeviation);
			break;
		}
		case '[': {
			// Save current position, direction, and width
			positionStack.push(position);
			directionStack.push(direction);
			widthStack.push(currentWidth);
			branchLengthStack.push(currentBranchLength);
			break;
		}
		case ']': {
			// Place leaves at the end of the branch before restoring
			AddCanopy(BlockUpdates, system.LeafSize, position.X, position.Y, position.Z, system.LeafBlock, 200);

			// Restore saved position, direction, and width
			if (!positionStack.empty()) {
				position = positionStack.top();
				positionStack.pop();
			}
			if (!directionStack.empty()) {
				direction = directionStack.top();
				directionStack.pop();
			}
			if (!widthStack.empty()) {
				currentWidth = widthStack.top();
				widthStack.pop();
			}
			if (!branchLengthStack.empty()) {
				currentBranchLength = branchLengthStack.top();
				branchLengthStack.pop();
			}
			break;
		}
		default:
			break;
		}
	}
}

void AProceduralTerrain::GenerateTestTreeAtLocation(TArray<FCachedBlockUpdate>& BlockUpdates, UChunkClass* Chunk, int x, int y, int z) {
	FTreeSystem treeSystem;
	treeSystem.Axiom = "F";
	treeSystem.Rules.Add('F', "/F-F-F\\F\\F+F+F\\F");
	treeSystem.Iterations = 3;
	//treeSystem.Rules.Add('F', "F[&F][^F]");
	treeSystem.Angle = 90;
	treeSystem.AngleDeviation = 1;
	treeSystem.BaseBranchLength = 6;  // Define the base branch length here
	treeSystem.MinBranchLength = 6;  // Define the base branch length here
	treeSystem.TrunkDeviation = 0;
	treeSystem.BaseTrunkWidth = 2;   // Define the base trunk radius here
	treeSystem.MinBranchWidth = 2;   // Define the base trunk radius here
	treeSystem.LeafSize = 5;
	treeSystem.LeafBlock = EBlock::Leaves;
	treeSystem.WoodBlock = EBlock::CliffStone;

	GenerateTree(BlockUpdates, Chunk, x, y, z, treeSystem);  // 5 iterations for tree generation
}