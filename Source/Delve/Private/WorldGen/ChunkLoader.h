#pragma once

#include "CoreMinimal.h"
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "../Utils/Enums.h"
#include "../Utils/ChunkStructs.h"
#include <exception>
#include <stdexcept>

// Define the save and load path
#define SAVE_LOAD_PATH FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("chunkData.dlv"))

struct ChunkSaveData {
    FIntVector ChunkPosition;
    std::unordered_map<uint8_t, uint8_t> BlockTypeMap;
    int BitsNeeded;
    std::vector<uint8_t> CompressedBlocks;
};

struct LoadFailedException : public std::exception
{
private:
    std::string errorMessage;

public:
    LoadFailedException(const std::string& msg) : errorMessage(msg) {}

    const char* what() const throw ()
    {
        return errorMessage.c_str();
    }
};

class ChunkLoader
{
public:
    ChunkLoader() = delete;
    ~ChunkLoader() = delete;

    static int LoadAllChunks(TArray<FChunkData>& chunks);
    static void SaveAllChunks(const TMap<FIntVector, TSharedPtr<FChunkData>>& ActiveChunkMap);
    static void SaveChunkToLine(const FChunkData& ChunkData, int32 LineNumber);
    static FChunkData LoadChunkAtLine(int32 LineNumber);

private:
    static FString GetFilePath();
    static std::pair<std::unordered_map<uint8_t, uint8_t>, int> CreateBlockTypeMapping(const std::vector<uint8_t>& blockArray);
    static std::vector<uint8_t> CompressBlocks(const std::vector<uint8_t>& blockArray, const std::unordered_map<uint8_t, uint8_t>& blockTypeMap, int bitsNeeded);
    static std::vector<uint8_t> DecompressBlocks(const std::vector<uint8_t>& compressedArray, const std::unordered_map<uint8_t, uint8_t>& reverseBlockTypeMap, int bitsNeeded);
    static ChunkSaveData CompressChunk(const std::vector<uint8_t>& blockArray, FIntVector chunkPosition);
    static std::string SerializeChunkData(const ChunkSaveData& chunkData);
    static ChunkSaveData DeserializeChunkData(const std::string& line);
    static std::vector<uint8_t> ConvertEnumArrayToUInt8(const TArray<EBlock>& enumArray);
    static TArray<EBlock> ConvertUInt8ToEnumArray(const std::vector<uint8_t>& uint8Array);
    static void SaveChunkDataToFile(const std::string& data, const std::string& path);
    static ChunkSaveData LoadChunkDataFromFile(const std::string& filePath);
};
