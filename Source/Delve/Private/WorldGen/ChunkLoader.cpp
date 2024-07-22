#include "ChunkLoader.h"

FString ChunkLoader::GetFilePath()
{
    return SAVE_LOAD_PATH;
}

int ChunkLoader::LoadAllChunks(TArray<FChunkData>& chunks)
{
    TArray<FString> Lines;
    if (!FFileHelper::LoadFileToStringArray(Lines, *GetFilePath()))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load chunk data from file."));
        return -1;
    }
    int32 LineNumber = 0;

    /*int32 TotalLines = Lines.Num();
    FThreadSafeCounter Counter(TotalLines);
    FEvent* AllOperationsCompleteEvent = FPlatformProcess::GetSynchEventFromPool(true);
    chunks.SetNumUninitialized(TotalLines);*/

    for (const FString& Line : Lines)
    {
        //AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [&chunks, &LineNumber, &Counter, &AllOperationsCompleteEvent]()
           // {
                //try
                {
                    //UE_LOG(LogTemp, Error, TEXT("Loading Chunk on line %d."), LineNumber);
                    ChunkSaveData chunkSaveData = DeserializeChunkData(TCHAR_TO_UTF8(*Line));
                    FChunkData chunkData;
                    chunkData.Position = chunkSaveData.ChunkPosition;
                    chunkData.Blocks = ConvertUInt8ToEnumArray(DecompressBlocks(chunkSaveData.CompressedBlocks, chunkSaveData.BlockTypeMap, chunkSaveData.BitsNeeded));
                    chunkData.line = LineNumber;
                    chunkData.GenerationLayer = ECompletedGenerationLayer::CompleteInActive;

                    //UE_LOG(LogTemp, Error, TEXT("Loaded Chunk %d.%d.%d with %d Blocks from line %d."), chunkData.Position.X, chunkData.Position.Y, chunkData.Position.Z, chunkData.Blocks.Num(), LineNumber);
                    chunks.Add(chunkData);
                    LineNumber++;
                }
                /*catch (const LoadFailedException& e)
                {
                    UE_LOG(LogTemp, Error, TEXT("Error deserializing chunk data: %s"), *FString(e.what()));
                    return -1;
                }*/
                /*if (Counter.Decrement() == 0)
                {
                    AllOperationsCompleteEvent->Trigger();
                }*/
            //});

        //AllOperationsCompleteEvent->Wait();
    }

    return 0;
}

void ChunkLoader::SaveAllChunks(const TMap<FIntVector, TSharedPtr<FChunkData>>& ActiveChunkMap)
{
    TArray<FString> Lines;

    for (const auto& Elem : ActiveChunkMap)
    {
        const TSharedPtr<FChunkData>& ChunkData = Elem.Value;
        std::vector<uint8_t> blockArray = ConvertEnumArrayToUInt8(ChunkData->Blocks);
        ChunkSaveData chunkSaveData = CompressChunk(blockArray, ChunkData->Position);
        FString Line = FString(UTF8_TO_TCHAR(SerializeChunkData(chunkSaveData).c_str()));
        Lines.Add(Line);
    }

    if (!FFileHelper::SaveStringArrayToFile(Lines, *GetFilePath()))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save chunk data to file."));
    }
}

void ChunkLoader::SaveChunkToLine(const FChunkData& ChunkData, int32 LineNumber)
{
    TArray<FString> Lines;
    if (!FFileHelper::LoadFileToStringArray(Lines, *GetFilePath()))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load chunk data from file."));
        return;
    }

    if (Lines.IsValidIndex(LineNumber))
    {
        std::vector<uint8_t> blockArray = ConvertEnumArrayToUInt8(ChunkData.Blocks);
        ChunkSaveData chunkSaveData = CompressChunk(blockArray, ChunkData.Position);
        FString Line = FString(UTF8_TO_TCHAR(SerializeChunkData(chunkSaveData).c_str()));
        Lines[LineNumber] = Line;
        if (!FFileHelper::SaveStringArrayToFile(Lines, *GetFilePath()))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to save chunk data to file."));
        }
    }
}

FChunkData ChunkLoader::LoadChunkAtLine(int32 LineNumber)
{
    TArray<FString> Lines;
    if (!FFileHelper::LoadFileToStringArray(Lines, *GetFilePath()))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load chunk data from file."));
        throw LoadFailedException("Failed to load chunk data from file.");
    }

    if (!Lines.IsValidIndex(LineNumber))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid line number."));
        throw LoadFailedException("Invalid line number.");
    }

    //try
    {
        ChunkSaveData chunkSaveData = DeserializeChunkData(TCHAR_TO_UTF8(*Lines[LineNumber]));
        FChunkData chunkData;
        chunkData.Position = chunkSaveData.ChunkPosition;
        chunkData.Blocks = ConvertUInt8ToEnumArray(DecompressBlocks(chunkSaveData.CompressedBlocks, chunkSaveData.BlockTypeMap, chunkSaveData.BitsNeeded));
        chunkData.line = LineNumber;
        return chunkData;
    }
    /*catch (const LoadFailedException& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Error deserializing chunk data: %s"), *FString(e.what()));
        throw;
    }*/
}

std::pair<std::unordered_map<uint8_t, uint8_t>, int> ChunkLoader::CreateBlockTypeMapping(const std::vector<uint8_t>& blockArray)
{
    std::unordered_map<uint8_t, uint8_t> blockTypeMap;
    uint8_t nextCompressedValue = 0;

    for (uint8_t block : blockArray)
    {
        if (blockTypeMap.find(block) == blockTypeMap.end())
        {
            blockTypeMap[block] = nextCompressedValue++;
        }
    }

    int bitsNeeded = std::ceil(std::log2(nextCompressedValue));
    return { blockTypeMap, bitsNeeded };
}

std::vector<uint8_t> ChunkLoader::CompressBlocks(const std::vector<uint8_t>& blockArray, const std::unordered_map<uint8_t, uint8_t>& blockTypeMap, int bitsNeeded)
{
    std::vector<uint8_t> compressed;

    // Check if the entire block array is composed of a single block type
    if (std::all_of(blockArray.begin(), blockArray.end(), [firstBlock = blockArray[0]](uint8_t block) { return block == firstBlock; }))
    {
        // Insert a special marker (e.g., 0xFF) to indicate a uniform block array
        compressed.push_back(0xFF);
        // Insert the compressed value of the block type
        compressed.push_back(blockTypeMap.at(blockArray[0]));
        return compressed;
    }

    uint8_t currentByte = 0;
    int bitIndex = 0;

    for (uint8_t block : blockArray)
    {
        uint8_t compressedValue = blockTypeMap.at(block);
        for (int j = 0; j < bitsNeeded; ++j)
        {
            currentByte |= ((compressedValue >> j) & 1) << bitIndex++;
            if (bitIndex == 8)
            {
                compressed.push_back(currentByte);
                currentByte = 0;
                bitIndex = 0;
            }
        }
    }

    if (bitIndex > 0)
    {
        compressed.push_back(currentByte);
    }

    return compressed;
}

std::vector<uint8_t> ChunkLoader::DecompressBlocks(const std::vector<uint8_t>& compressedArray, const std::unordered_map<uint8_t, uint8_t>& reverseBlockTypeMap, int bitsNeeded)
{
    std::vector<uint8_t> decompressed;

    // Check for the special marker indicating a uniform block array
    if (compressedArray.size() == 2 && compressedArray[0] == 0xFF)
    {
        uint8_t compressedValue = compressedArray[1];
        if (reverseBlockTypeMap.find(compressedValue) == reverseBlockTypeMap.end())
        {
            throw LoadFailedException("Invalid compressed value encountered during decompression.");
        }
        uint8_t blockType = reverseBlockTypeMap.at(compressedValue);
        decompressed.resize(262144, blockType);  // Fill the entire decompressed array with the block type
        return decompressed;
    }

    uint8_t currentByte = compressedArray[0];
    int bitIndex = 0;
    int byteIndex = 1;

    while (byteIndex < compressedArray.size() || (byteIndex == compressedArray.size() && bitIndex < 8))
    {
        uint8_t compressedValue = 0;
        for (int j = 0; j < bitsNeeded; ++j)
        {
            if (bitIndex == 8)
            {
                bitIndex = 0;
                if (byteIndex < compressedArray.size())
                {
                    currentByte = compressedArray[byteIndex++];
                }
                else
                {
                    throw LoadFailedException("Reached the end of compressed array prematurely.");
                }
            }
            compressedValue |= ((currentByte >> bitIndex++) & 1) << j;
        }

        if (reverseBlockTypeMap.find(compressedValue) == reverseBlockTypeMap.end())
        {
            throw LoadFailedException("Invalid compressed value encountered during decompression.");
        }

        uint8_t blockType = reverseBlockTypeMap.at(compressedValue);
        decompressed.push_back(blockType);
    }

    return decompressed;
}

ChunkSaveData ChunkLoader::CompressChunk(const std::vector<uint8_t>& blockArray, FIntVector chunkPosition)
{
    auto [blockTypeMap, bitsNeeded] = CreateBlockTypeMapping(blockArray);

    std::vector<uint8_t> compressedBlocks = CompressBlocks(blockArray, blockTypeMap, bitsNeeded);

    ChunkSaveData chunkData = { chunkPosition, blockTypeMap, bitsNeeded, compressedBlocks };
    return chunkData;
}

std::string ChunkLoader::SerializeChunkData(const ChunkSaveData& chunkData)
{
    std::stringstream ss;
    ss << chunkData.ChunkPosition.X << " " << chunkData.ChunkPosition.Y << " " << chunkData.ChunkPosition.Z << " ";
    ss << chunkData.BitsNeeded << " ";

    for (const auto& pair : chunkData.BlockTypeMap)
    {
        ss << static_cast<int>(pair.first) << ":" << static_cast<int>(pair.second) << " ";
    }

    for (uint8_t byte : chunkData.CompressedBlocks)
    {
        ss << static_cast<int>(byte) << " ";
    }

    return ss.str();
}

ChunkSaveData ChunkLoader::DeserializeChunkData(const std::string& line)
{
    std::stringstream ss(line);
    std::string segment;

    FIntVector chunkPosition;
    ss >> chunkPosition.X >> chunkPosition.Y >> chunkPosition.Z;

    int bitsNeeded;
    ss >> bitsNeeded;

    std::unordered_map<uint8_t, uint8_t> blockTypeMap;
    while (ss >> segment)
    {
        if (segment.find(':') == std::string::npos) break;
        std::stringstream pairStream(segment);
        std::string key, value;
        std::getline(pairStream, key, ':');
        std::getline(pairStream, value, ':');
        blockTypeMap[std::stoi(key)] = std::stoi(value);
    }
    // Reverse the BlockTypeMap
    std::unordered_map<uint8_t, uint8_t> reverseBlockTypeMap;
    for (const auto& pair : blockTypeMap)
    {
        reverseBlockTypeMap[pair.second] = pair.first;
    }
    blockTypeMap = reverseBlockTypeMap;

    std::vector<uint8_t> compressedBlocks;
    do
    {
        //try
        {
            compressedBlocks.push_back(static_cast<uint8_t>(std::stoi(segment)));
        }
        /*catch (const std::exception& e)
        {
            UE_LOG(LogTemp, Error, TEXT("Error parsing compressed block: %s"), *FString(e.what()));
            break;
        }*/
    } while (ss >> segment);

    ChunkSaveData chunkData = { chunkPosition, blockTypeMap, bitsNeeded, compressedBlocks };
    return chunkData;
}

std::vector<uint8_t> ChunkLoader::ConvertEnumArrayToUInt8(const TArray<EBlock>& enumArray)
{
    std::vector<uint8_t> uint8Array;
    uint8Array.reserve(enumArray.Num());

    for (const auto& block : enumArray)
    {
        uint8Array.push_back(static_cast<uint8_t>(block));
    }

    return uint8Array;
}

TArray<EBlock> ChunkLoader::ConvertUInt8ToEnumArray(const std::vector<uint8_t>& uint8Array)
{
    TArray<EBlock> enumArray;
    enumArray.Reserve(uint8Array.size());

    for (uint8_t value : uint8Array)
    {
        enumArray.Add(static_cast<EBlock>(value));
    }

    return enumArray;
}

void ChunkLoader::SaveChunkDataToFile(const std::string& data, const std::string& path)
{
    std::ofstream outFile(path);
    if (outFile.is_open())
    {
        outFile << data << std::endl;
        outFile.close();
        UE_LOG(LogTemp, Warning, TEXT("Saved chunkData.dlv to: %s"), *FString(path.c_str()));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Error: Unable to open file for writing: %s"), *FString(path.c_str()));
    }
}

ChunkSaveData ChunkLoader::LoadChunkDataFromFile(const std::string& filePath)
{
    std::ifstream inFile(filePath);
    if (!inFile.is_open())
    {
        UE_LOG(LogTemp, Error, TEXT("Error: Unable to open file for reading: %s"), *FString(filePath.c_str()));
        throw LoadFailedException("Error: Unable to open file for reading: " + filePath);
    }

    std::string line;
    std::getline(inFile, line);
    inFile.close();

    if (line.empty())
    {
        UE_LOG(LogTemp, Error, TEXT("Error: File is empty or reading error occurred."));
        throw LoadFailedException("Error: File is empty or reading error occurred.");
    }

    UE_LOG(LogTemp, Log, TEXT("Serialized Data: %s"), *FString(line.c_str()));

    ChunkSaveData loadedChunk = ChunkLoader::DeserializeChunkData(line);
    return loadedChunk;
}
