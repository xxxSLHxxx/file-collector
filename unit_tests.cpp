#pragma once
#include <gtest/gtest.h>
#include <vector>
#include "Sender.h"

TEST(FileCollectorTest, CollectFileCreatesFileEntry) {
    FileCollector collector;

    uint32_t fileId = 1;
    size_t fileSize = 100;

    collector.CollectFile(fileId, fileSize);

    std::vector<uint8_t> file = collector.GetFile(fileId);
    EXPECT_EQ(file.size(), fileSize);
}

TEST(FileCollectorTest, OnNewChunkAddsChunksCorrectly) {
    FileCollector collector;

    uint32_t fileId = 1;
    size_t fileSize = 100;
    collector.CollectFile(fileId, fileSize);

    Chunk chunk1(10, 1);  
    collector.OnNewChunk(fileId, 0, chunk1); 

    std::vector<uint8_t> file = collector.GetFile(fileId);
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_EQ(file[i], 1);
    }

    Chunk chunk2(10, 2);
    collector.OnNewChunk(fileId, 10, chunk2);

    file = collector.GetFile(fileId);
    for (size_t i = 10; i < 20; ++i) {
        EXPECT_EQ(file[i], 2);
    }
}

TEST(FileCollectorTest, OnNewChunkHandlesDuplicateChunks) {
    FileCollector collector;

    uint32_t fileId = 1;
    size_t fileSize = 100;
    collector.CollectFile(fileId, fileSize);

    Chunk chunk1(10, 1);
    collector.OnNewChunk(fileId, 0, chunk1);  

    collector.OnNewChunk(fileId, 0, chunk1);

    std::vector<uint8_t> file = collector.GetFile(fileId);
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_EQ(file[i], 1);
    }
}

TEST(FileCollectorTest, GetFileHandlesNonExistentFile) {
    FileCollector collector;

    uint32_t nonExistentFileId = 999;
    std::vector<uint8_t> file = collector.GetFile(nonExistentFileId);
    EXPECT_TRUE(file.empty());
}

TEST(FileCollectorTest, LargeDataTest) {
    FileCollector collector;

    uint32_t fileId = 1;
    size_t fileSize = 1000000;
    collector.CollectFile(fileId, fileSize);

    Chunk chunk(1000, 1); 
    for (size_t i = 0; i < fileSize / chunk.size(); ++i) {
        collector.OnNewChunk(fileId, i * chunk.size(), chunk);
    }

    std::vector<uint8_t> file = collector.GetFile(fileId);
    EXPECT_EQ(file.size(), fileSize);

    for (size_t i = 0; i < file.size(); ++i) {
        EXPECT_EQ(file[i], 1);
    }
}

TEST(FileCollectorBenchmark, PerformanceTestSingleThreaded) {
    FileCollector collector;

    const uint32_t fileId = 42;
    const size_t fileSize = 10'000'000;
    const size_t chunkSize = 1000;

    collector.CollectFile(fileId, fileSize);
    Chunk chunk(chunkSize, 1);

    for (size_t offset = 0; offset < fileSize; offset += chunkSize) {
        collector.OnNewChunk(fileId, offset, chunk); 
    }

    auto start = std::chrono::high_resolution_clock::now();
    const auto file = collector.GetFile(fileId);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "[ BENCHMARK ] GetFile took " << duration_ms << " ms\n";
    EXPECT_EQ(file.size(), fileSize);
    for (size_t i = 0; i < fileSize; ++i) {
        EXPECT_EQ(file[i], 1);
    }
}

