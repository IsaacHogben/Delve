// Fill out your copyright notice in the Description page of Project Settings.

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
#include "../Utils/Enums.h"

// Define the save and load path
#define SAVE_LOAD_PATH (std::filesystem::temp_directory_path() / "chunkData.txt").string()

struct ChunkSaveData {
    FIntVector ChunkPosition;
    std::unordered_map<uint8_t, uint8_t> BlockTypeMap;
    int BitsNeeded;
    std::vector<uint8_t> CompressedBlocks;
};

#include <exception>
#include <stdexcept>

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

/**
 *
 */
class ChunkLoader
{
public:
    ChunkLoader();
    ~ChunkLoader();

    // Function to create block type mapping and determine number of bits needed
    static std::pair<std::unordered_map<uint8_t, uint8_t>, int> CreateBlockTypeMapping(const std::vector<uint8_t>& blockArray)
    {
        std::unordered_map<uint8_t, uint8_t> blockTypeMap;
        uint8_t nextCompressedValue = 0;

        for (uint8_t block : blockArray) {
            if (blockTypeMap.find(block) == blockTypeMap.end()) {
                blockTypeMap[block] = nextCompressedValue++;
            }
        }

        int bitsNeeded = std::ceil(std::log2(nextCompressedValue));
        return { blockTypeMap, bitsNeeded };
    }

    static std::vector<uint8_t> CompressBlocks(const std::vector<uint8_t>& blockArray, const std::unordered_map<uint8_t, uint8_t>& blockTypeMap, int bitsNeeded)
    {
        std::vector<uint8_t> compressed;
        uint8_t currentByte = 0;
        int bitIndex = 0;
        int runLength = 0;
        uint8_t lastBlock = blockArray[0];

        for (size_t i = 0; i <= blockArray.size(); ++i) {
            if (i < blockArray.size() && blockArray[i] == lastBlock) {
                ++runLength;
                continue;
            }

            // Write the block type
            uint8_t compressedValue = blockTypeMap.at(lastBlock);
            for (int j = 0; j < bitsNeeded; ++j) {
                currentByte |= ((compressedValue >> j) & 1) << bitIndex++;
                if (bitIndex == 8) {
                    compressed.push_back(currentByte);
                    currentByte = 0;
                    bitIndex = 0;
                }
            }

            // Write the run length
            for (int j = 0; j < 8; ++j) {
                currentByte |= ((runLength >> j) & 1) << bitIndex++;
                if (bitIndex == 8) {
                    compressed.push_back(currentByte);
                    currentByte = 0;
                    bitIndex = 0;
                }
            }

            if (i < blockArray.size()) {
                lastBlock = blockArray[i];
                runLength = 1;
            }
        }

        if (bitIndex > 0) {
            compressed.push_back(currentByte);
        }

        return compressed;
    }

    static std::vector<uint8_t> DecompressBlocks(const std::vector<uint8_t>& compressedArray, const std::unordered_map<uint8_t, uint8_t>& reverseBlockTypeMap, int bitsNeeded)
    {
        std::vector<uint8_t> decompressed;
        uint8_t currentByte = compressedArray[0];
        int bitIndex = 0;
        int byteIndex = 1;

        while (byteIndex <= compressedArray.size()) {
            uint8_t compressedValue = 0;
            for (int j = 0; j < bitsNeeded; ++j) {
                if (bitIndex == 8) {
                    bitIndex = 0;
                    currentByte = compressedArray[byteIndex++];
                }
                compressedValue |= ((currentByte >> bitIndex++) & 1) << j;
            }

            uint8_t blockType = reverseBlockTypeMap.at(compressedValue);

            int runLength = 0;
            for (int j = 0; j < 8; ++j) {
                if (bitIndex == 8) {
                    bitIndex = 0;
                    currentByte = compressedArray[byteIndex++];
                }
                runLength |= ((currentByte >> bitIndex++) & 1) << j;
            }

            decompressed.insert(decompressed.end(), runLength, blockType);
        }

        return decompressed;
    }

    static ChunkSaveData CompressChunk(const std::vector<uint8_t>& blockArray, FIntVector chunkPosition) {
        auto [blockTypeMap, bitsNeeded] = CreateBlockTypeMapping(blockArray);

        // Create a reverse map for decompression
        std::unordered_map<uint8_t, uint8_t> reverseBlockTypeMap;
        for (const auto& pair : blockTypeMap) {
            reverseBlockTypeMap[pair.second] = pair.first;
        }

        std::vector<uint8_t> compressedBlocks = CompressBlocks(blockArray, blockTypeMap, bitsNeeded);

        ChunkSaveData chunkData = { chunkPosition, blockTypeMap, bitsNeeded, compressedBlocks };
        return chunkData;
    };

    static std::string SerializeChunkData(const ChunkSaveData& chunkData) {
        std::stringstream ss;
        ss << chunkData.ChunkPosition.X << " " << chunkData.ChunkPosition.Y << " " << chunkData.ChunkPosition.Z << " ";

        ss << chunkData.BitsNeeded << " ";

        for (const auto& pair : chunkData.BlockTypeMap) {
            ss << static_cast<int>(pair.first) << ":" << static_cast<int>(pair.second) << " ";
        }

        for (uint8_t byte : chunkData.CompressedBlocks) {
            ss << static_cast<int>(byte) << " ";
        }

        return ss.str();
    };

    static ChunkSaveData DeserializeChunkData(const std::string& line) {
        std::stringstream ss(line);
        std::string segment;

        // Deserialize chunk position
        FIntVector chunkPosition;
        ss >> chunkPosition.X >> chunkPosition.Y >> chunkPosition.Z;

        // Deserialize bits needed
        int bitsNeeded;
        ss >> bitsNeeded;

        // Deserialize block type map
        std::unordered_map<uint8_t, uint8_t> blockTypeMap;
        while (ss >> segment) {
            if (segment.find(':') == std::string::npos) break;
            std::stringstream pairStream(segment);
            std::string key, value;
            std::getline(pairStream, key, ':');
            std::getline(pairStream, value, ':');
            blockTypeMap[std::stoi(key)] = std::stoi(value);
        }

        // Deserialize compressed blocks
        std::vector<uint8_t> compressedBlocks;
        do {
            try {
                compressedBlocks.push_back(static_cast<uint8_t>(std::stoi(segment)));
            }
            catch (const std::exception& e) {
                UE_LOG(LogTemp, Error, TEXT("Error parsing compressed block: %s"), *FString(e.what()));
                break;
            }
        } while (ss >> segment);

        ChunkSaveData chunkData = { chunkPosition, blockTypeMap, bitsNeeded, compressedBlocks };
        return chunkData;
    };

    static std::vector<uint8_t> ConvertEnumArrayToUInt8(const TArray<EBlock>& enumArray)
    {
        std::vector<uint8_t> uint8Array;
        uint8Array.reserve(enumArray.Num());

        for (const auto& block : enumArray) {
            uint8Array.push_back(static_cast<uint8_t>(block));
        }

        return uint8Array;
    };

    static TArray<EBlock> ConvertUInt8ToEnumArray(const std::vector<uint8_t>& uint8Array)
    {
        TArray<EBlock> enumArray;
        enumArray.Reserve(uint8Array.size());

        for (uint8_t value : uint8Array) {
            enumArray.Add(static_cast<EBlock>(value));
        }

        return enumArray;
    }

    static void SaveChunkDataToFile(const std::string& data, const std::string& path)
    {
        // Save data to file
        std::ofstream outFile(path);
        if (outFile.is_open()) {
            outFile << data << std::endl;
            outFile.close();
            FString fp(path.c_str());
            UE_LOG(LogTemp, Warning, TEXT("Saved chunkData.txt to: %s"), *fp);
        }
        else {
            UE_LOG(LogTemp, Warning, TEXT("Error: Unable to open file for writing: "));
        }
    };

    static ChunkSaveData LoadChunkDataFromFile(const std::string& filePath)
    {
        // Open the file for reading
        std::ifstream inFile(filePath);
        if (!inFile.is_open()) {
            UE_LOG(LogTemp, Error, TEXT("Error: Unable to open file for reading: %s"), *FString(filePath.c_str()));
            throw LoadFailedException("Error: Unable to open file for reading: " + filePath);
        }

        // Read the serialized data from the file
        std::string line;
        std::getline(inFile, line);
        inFile.close();

        // Check if the line is empty
        if (line.empty()) {
            UE_LOG(LogTemp, Error, TEXT("Error: File is empty or reading error occurred."));
            throw LoadFailedException("Error: File is empty or reading error occurred.");
        }

        // Log the serialized data for debugging
        UE_LOG(LogTemp, Log, TEXT("Serialized Data: %s"), *FString(line.c_str()));

        // Deserialize the serialized data into a ChunkSaveData structure
        ChunkSaveData loadedChunk = ChunkLoader::DeserializeChunkData(line);
        
        return loadedChunk;
    };

    static int SaveChunk(TArray<EBlock>& blockEnumArray, FIntVector chunkPosition)
    {
        // Convert enum array to uint8 array
        std::vector<uint8_t> blockArray = ConvertEnumArrayToUInt8(blockEnumArray);

        // Compress the chunk data
        ChunkSaveData compressedChunk = ChunkLoader::CompressChunk(blockArray, chunkPosition);

        // Serialize the compressed chunk data to a string
        std::string serializedData = ChunkLoader::SerializeChunkData(compressedChunk);

        // Write the serialized data to the file
        SaveChunkDataToFile(serializedData, SAVE_LOAD_PATH);

        // Return success or appropriate error code
        return 0;
    };

    static int LoadChunk(TArray<EBlock>& blockEnumArray, FIntVector& chunkPosition)
    {
        ChunkSaveData loadedChunk = LoadChunkDataFromFile(SAVE_LOAD_PATH);

        // Create a reverse map of block type mappings for decompression
        std::unordered_map<uint8_t, uint8_t> reverseBlockTypeMap;
        for (const auto& pair : loadedChunk.BlockTypeMap) {
            reverseBlockTypeMap[pair.second] = pair.first;
        }

        // Decompress the blocks using the reverse block type map
        std::vector<uint8_t> decompressedBlocks = ChunkLoader::DecompressBlocks(loadedChunk.CompressedBlocks, reverseBlockTypeMap, loadedChunk.BitsNeeded);

        // Log compressed blocks for debugging
        FString compressedBlocksStr;
        for (uint8_t block : loadedChunk.CompressedBlocks) {
            compressedBlocksStr += FString::Printf(TEXT("%d "), block);
        }
        UE_LOG(LogTemp, Log, TEXT("Compressed Blocks: %s"), *compressedBlocksStr);

        // Log decompressed blocks for debugging
        FString decompressedBlocksStr;
        for (uint8_t block : decompressedBlocks) {
            decompressedBlocksStr += FString::Printf(TEXT("%d "), block);
        }
        UE_LOG(LogTemp, Log, TEXT("Decompressed Blocks: %s"), *decompressedBlocksStr);

        blockEnumArray = ConvertUInt8ToEnumArray(decompressedBlocks);
        chunkPosition = loadedChunk.ChunkPosition;

        return 0;
    }
};
