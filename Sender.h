#pragma once
#include"FileCollector.h"
using fileVector = std::vector<uint8_t>;
// Класс имитирующий отправителя в сети, от которого поступают чанки
class Sender {
public:
	void SendFile(fileVector file, uint32_t fileId);
	void SendManyFiles(const std::unordered_map<uint32_t, fileVector>& idToFile);
	std::vector<std::pair<size_t, Chunk>> SplitIntoChunks(const fileVector& file, size_t chunkCount, bool sameSize);
	//void Benchmark(uint32_t fileId, const std::vector<uint8_t>& file, uint32_t chunkNum);
	//void BenchmarkConcurrentVsSingle(const std::unordered_map<uint32_t, fileVector>& files);
	void ResetCollector();
	FileCollector* GetCollector() { return &collector; }
private:
	FileCollector collector;
	std::mutex coutMx;
};