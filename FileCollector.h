#pragma once
#include<unordered_map>
#include<mutex>
#include<random>
#include <algorithm>
#include<future>
#include<iostream> 
#include<thread>
#include<future>
#include"File.h"
using Chunk = std::vector<uint8_t>;

class FileCollector
{
public:
    void CollectFile(uint32_t fileId, size_t fileSize);
    void RemoveFile(uint32_t fileId);

    void OnNewChunk( uint32_t fileId, size_t pos, Chunk& chunk);
   
    std::vector<uint8_t>& GetFile(uint32_t fileId);
    const std::vector<uint8_t>& GetFileReadOnly(uint32_t fileId);
    void Reset();

    std::mutex idToFileMutex;

private: 
    std::map<uint32_t, File> idToFile;
    std::unordered_map<uint32_t, std::mutex> fileMutexes;
};
