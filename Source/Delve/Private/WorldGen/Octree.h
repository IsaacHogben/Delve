// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// Represents a point in 3D space
struct FPoint {
    uint8 X, Y, Z;

    FPoint operator+(const FPoint& Other) const
    {
        return FPoint(X + Other.X, Y + Other.Y, Z + Other.Z);
    }
    FPoint operator-(const FPoint& Other) const
    {
        return FPoint(X - Other.X, Y - Other.Y, Z - Other.Z);
    }
    FPoint operator-(const int& Num) const
    {
        return FPoint(X - Num, Y - Num, Z - Num);
    }
    FPoint operator/(const float& Num) const
    {
        return FPoint(X / Num, Y / Num, Z / Num);
    }
    bool operator==(const FPoint& Other) const
    {
        return X == Other.X && Y == Other.Y && Z == Other.Z;
    }
};

// Represents a region in 3D space
struct FRegion {
    FPoint Min, Max;

    bool operator==(const FRegion& Other) const
    {
        return Min == Other.Min && Max == Other.Max;
    }
};

// Node of the octree
template<typename T>
class FOctreeNode {
public:
    FOctreeNode(const FRegion& Region) : Region(Region), IsLeaf(true) {}

    FRegion Region;
    bool IsLeaf;
    T Value;
    TMap<uint8, TUniquePtr<FOctreeNode<T>>> Children;
};

// Octree class
template<typename T>
class FOctree {
public:
    FOctree(const FRegion& Region, const T DefualtValue) : Root(MakeUnique<FOctreeNode<T>>(Region)), DefaultValue(DefaultValue){}

    T DefaultValue;

    // Insert a value at the specified point in space
    void Insert(const FPoint& Point, const T& Value) {
        Insert(Point, Value, Root.Get());
    }
    // Retrieve values within a given region
    TArray<T> QueryRegion(const FRegion& QueryRegion) const {
        TArray<T> Result;
        Query_Region(QueryRegion, Root.Get(), Result);
        return Result;
    }
    // Retrieve value at a point
    T QueryPoint(const FPoint& QueryPoint) const {
        T Result = DefaultValue;
        Query_Point(FRegion(QueryPoint - 1, QueryPoint), Root.Get(), Result);
        return Result;
    }

private:
    // Private member functions for recursive operations
    void Insert(const FPoint& Point, const T& Value, FOctreeNode<T>* Node) {
        //UE_LOG(LogTemp, Warning, TEXT("region min %d.%d.%d"), Node->Region.Min.X, Node->Region.Min.Y, Node->Region.Min.Z);
        //UE_LOG(LogTemp, Warning, TEXT("region max %d.%d.%d"), Node->Region.Max.X, Node->Region.Max.Y, Node->Region.Max.Z);
        if (!IsPointInsideRegion(Point, Node->Region)) {
            return;
        }

        if (Node->IsLeaf && !IsMaxDepthReached(Node->Region)) {
            //Node->Value = Value;
            Node->IsLeaf = false;
            CreateChildren(Node);
        }
        else
        {
            Node->Value = Value;
            UE_LOG(LogTemp, Warning, TEXT("region max"));
            return;
        }
        
        uint8 ChildIndex = GetChildIndex(Point, Node->Region);
        Insert(Point, Value, Node->Children.FindOrAdd(ChildIndex).Get());
    }

    void Query_Region(const FRegion& QueryRegion, const FOctreeNode<T>* Node, TArray<T>& Result) const {
        if (!IsRegionOverlap(QueryRegion, Node->Region)) {
            return;
        }

        if (Node->IsLeaf) {
            Result.Add(Node->Value);
        } else {
            // Recursively query child nodes
            for (const auto& Child : Node->Children) {
                Query_Region(QueryRegion, Child.Value.Get(), Result);
            }
        }
    }

    int Query_Point(const FRegion& QueryRegion, const FOctreeNode<T>* Node, T& Result) const {
        if (!IsRegionOverlap(QueryRegion, Node->Region)) {
            return 0;
        }

        if (Node->IsLeaf && QueryRegion == Node->Region) {
            Result = Node->Value;
            //UE_LOG(LogTemp, Warning, TEXT("value retreived"));
            return 1;
        }
        else {
            // Recursively query child nodes
            for (const auto& Child : Node->Children) {
                if(Query_Point(QueryRegion, Child.Value.Get(), Result))
                    return 1;
            }
        }
        return 0;
    }

    bool IsMaxDepthReached(const FRegion& Region) const
    {
        return (Region.Max.X - Region.Min.X) <= 1 && (Region.Max.Y - Region.Min.Y) <= 1 && (Region.Max.Z - Region.Min.Z) <= 1;
    }

    bool IsPointInsideRegion(const FPoint& Point, const FRegion& Region) const {
        return Point.X >= Region.Min.X && Point.X <= Region.Max.X &&
               Point.Y >= Region.Min.Y && Point.Y <= Region.Max.Y &&
               Point.Z >= Region.Min.Z && Point.Z <= Region.Max.Z;
    }

    bool IsRegionOverlap(const FRegion& Region1, const FRegion& Region2) const {
        return Region1.Min.X <= Region2.Max.X && Region1.Max.X >= Region2.Min.X &&
               Region1.Min.Y <= Region2.Max.Y && Region1.Max.Y >= Region2.Min.Y &&
               Region1.Min.Z <= Region2.Max.Z && Region1.Max.Z >= Region2.Min.Z;
    }

    uint8 GetChildIndex(const FPoint& Point, const FRegion& Region) const {
        uint8 Index = 0;
        Index |= (Point.X > (Region.Min.X + Region.Max.X) / 2.0f) ? 1 : 0;
        Index |= (Point.Y > (Region.Min.Y + Region.Max.Y) / 2.0f) ? 2 : 0;
        Index |= (Point.Z > (Region.Min.Z + Region.Max.Z) / 2.0f) ? 4 : 0;
        return Index;
    }

    void CreateChildren(FOctreeNode<T>* Node) {
        const FPoint MidPoint = (Node->Region.Max + Node->Region.Min) / 2.0f;
        for (int32 i = 0; i < 8; ++i) {
            FRegion ChildRegion;
            ChildRegion.Min.X = (i & 1) ? MidPoint.X : Node->Region.Min.X;
            ChildRegion.Max.X = (i & 1) ? Node->Region.Max.X : MidPoint.X;
            ChildRegion.Min.Y = (i & 2) ? MidPoint.Y : Node->Region.Min.Y;
            ChildRegion.Max.Y = (i & 2) ? Node->Region.Max.Y : MidPoint.Y;
            ChildRegion.Min.Z = (i & 4) ? MidPoint.Z : Node->Region.Min.Z;
            ChildRegion.Max.Z = (i & 4) ? Node->Region.Max.Z : MidPoint.Z;

            Node->Children.Add(i, MakeUnique<FOctreeNode<T>>(ChildRegion));
            //UE_LOG(LogTemp, Warning, TEXT("region min %d.%d.%d"), ChildRegion.Min.X, ChildRegion.Min.Y, ChildRegion.Min.Z);
            //UE_LOG(LogTemp, Warning, TEXT("region max %d.%d.%d"), ChildRegion.Max.X, ChildRegion.Max.Y, ChildRegion.Max.Z);
        }
    }

    TUniquePtr<FOctreeNode<T>> Root;
};
