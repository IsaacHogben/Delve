// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGen/ChunkRenderDistance.h"

ChunkRenderDistance::ChunkRenderDistance()
{

}

void ChunkRenderDistance::CalculateRenderSphere()
{
	TArray<FIntVector> RenderSphere;
	int MaxRadius = DrawDistance;
	int MinRadius = 0;

	FIntVector PreviousVectorPos;
	const int PhiResolution = 1; // Increase the resolution of Phi
	const int ThetaResolution = 1; // Increase the resolution of Theta
	for (int Radius = MinRadius; Radius <= MaxRadius; ++Radius)
	{
		//add differenciate for chunk prep and render distance.
		for (int Phi = 0; Phi <= 360; Phi += PhiResolution)
		{
			for (int Theta = 0; Theta <= 180; Theta += ThetaResolution)
			{
				// Calculate coordinates in spherical coordinates
				float X = Radius * sin(Theta) * cos(Phi);
				float Y = Radius * sin(Theta) * sin(Phi);
				float Z = Radius * cos(Theta);

				int IntX = static_cast<int>(X);
				int IntY = static_cast<int>(Y);
				int IntZ = static_cast<int>(Z);

				FIntVector VectorPos = FIntVector(IntX, IntY, IntZ);
				if (VectorPos == PreviousVectorPos)
					break;
				if (!RenderSphere.Contains(VectorPos))
					RenderSphere.Add(VectorPos);
				PreviousVectorPos = VectorPos;
			}
		}
	}
	RenderHemisphere.SetNum(6);
	for (const FIntVector chunk : RenderSphere)
	{
		if (!RenderSphere.Contains(FIntVector(chunk.X + 1, chunk.Y, chunk.Z)))
		{
			RenderHemisphere[static_cast<int32>(EDirection::North)].Add(FIntVector(chunk.X + 1, chunk.Y, chunk.Z));
			RenderHemisphere[static_cast<int32>(EDirection::North)].Add(FIntVector(chunk.X, chunk.Y, chunk.Z));
		}
		if (!RenderSphere.Contains(FIntVector(chunk.X - 1, chunk.Y, chunk.Z)))
		{
			RenderHemisphere[static_cast<int32>(EDirection::South)].Add(FIntVector(chunk.X - 1, chunk.Y, chunk.Z));
			RenderHemisphere[static_cast<int32>(EDirection::South)].Add(FIntVector(chunk.X, chunk.Y, chunk.Z));
		}
		if (!RenderSphere.Contains(FIntVector(chunk.X, chunk.Y + 1, chunk.Z)))
		{
			RenderHemisphere[static_cast<int32>(EDirection::East)].Add(FIntVector(chunk.X, chunk.Y + 1, chunk.Z));
			RenderHemisphere[static_cast<int32>(EDirection::East)].Add(FIntVector(chunk.X, chunk.Y, chunk.Z));
		}
		if (!RenderSphere.Contains(FIntVector(chunk.X, chunk.Y - 1, chunk.Z)))
		{
			RenderHemisphere[static_cast<int32>(EDirection::West)].Add(FIntVector(chunk.X, chunk.Y - 1, chunk.Z));
			RenderHemisphere[static_cast<int32>(EDirection::West)].Add(FIntVector(chunk.X, chunk.Y, chunk.Z));
		}
		if (!RenderSphere.Contains(FIntVector(chunk.X, chunk.Y, chunk.Z + 1)))
		{
			RenderHemisphere[static_cast<int32>(EDirection::Up)].Add(FIntVector(chunk.X, chunk.Y, chunk.Z + 1));
			RenderHemisphere[static_cast<int32>(EDirection::Up)].Add(FIntVector(chunk.X, chunk.Y, chunk.Z));
		}
		if (!RenderSphere.Contains(FIntVector(chunk.X, chunk.Y, chunk.Z - 1)))
		{
			RenderHemisphere[static_cast<int32>(EDirection::Down)].Add(FIntVector(chunk.X, chunk.Y, chunk.Z - 1));
			RenderHemisphere[static_cast<int32>(EDirection::Down)].Add(FIntVector(chunk.X, chunk.Y, chunk.Z));
		}
	}
}

ChunkRenderDistance::~ChunkRenderDistance()
{
}
