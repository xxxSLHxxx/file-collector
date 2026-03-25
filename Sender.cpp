#include"Sender.h"
#include <atomic>
#include <functional> 


void Sender::SendFile(fileVector file, uint32_t fileId) {

	std::vector<std::pair<size_t, Chunk>> chunks = SplitIntoChunks(file, 20, true);
	collector.CollectFile(fileId, file.size());

	std::vector<std::future<void>> futures;
	for (auto& pair : chunks) {
		auto offset = pair.first;
		auto& chunk = pair.second;
		futures.emplace_back(std::async(std::launch::async, [&collector = this->collector, fileId, offset, chunk = std::move(chunk)]() mutable {
			collector.OnNewChunk(fileId, offset, chunk);
			}));
	}

	for (auto& f : futures) {
		f.get();
	}
	return;
}

std::vector<std::pair<size_t, Chunk>> Sender::SplitIntoChunks(const fileVector& file, size_t chunkCount, bool sameSize) {
	std::vector<std::pair<size_t, Chunk>> result;
	if (file.empty() || chunkCount == 0) return result;

	size_t fileSize = file.size();
	std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<size_t> offsetDist(0, fileSize - 1);

	if (sameSize) {
		size_t chunkSize = fileSize / chunkCount;
		for (size_t i = 0; i < chunkCount; ++i) {
			size_t offset = i * chunkSize;
			if (offset >= fileSize) break;

			size_t actualSize = std::min(chunkSize, fileSize - offset);
			result.emplace_back(offset, Chunk(file.begin() + offset, file.begin() + offset + actualSize));
		}
	}
	else {
		std::uniform_int_distribution<size_t> sizeDist(1, fileSize / 2 + 1); 

		for (size_t i = 0; i < chunkCount; ++i) {
			size_t offset = offsetDist(rng);
			size_t maxChunkSize = fileSize - offset;
			if (maxChunkSize == 0) continue;

			size_t chunkSize = std::min(sizeDist(rng), maxChunkSize);
			result.emplace_back(offset, Chunk(file.begin() + offset, file.begin() + offset + chunkSize));
		}
	}

	return result;
}

void Sender::ResetCollector() {

    std::lock_guard<std::mutex> lock(collector.idToFileMutex);
    collector.Reset();
}

void Sender::SendManyFiles(const std::unordered_map<uint32_t, fileVector>& idToFile) {
    using namespace std::chrono;
    std::vector<std::future<void>> futures;
    //auto startAll = steady_clock::now();

    for (auto it = idToFile.begin(); it != idToFile.end(); ++it) {
        uint32_t fileId = it->first;
        fileVector file = it->second;

        futures.emplace_back(std::async(std::launch::async, [this, fileId, file]() {
            this->SendFile(file, fileId);
            }));
    }

    for (size_t i = 0; i < futures.size(); ++i) {
        futures[i].get();
    }

    //auto endAll = steady_clock::now();
    // auto total = duration_cast<milliseconds>(endAll - startAll).count();
    //std::cout << "All files processed in " << total << " ms" << std::endl;
}

//void Sender::BenchmarkConcurrentVsSingle(const std::unordered_map<uint32_t, fileVector>& files) {
//    using namespace std::chrono;
//
//    
//    ResetCollector();
//    std::cout << "\n=== Multi-threaded processing ===" << std::endl;
//    auto multi_start = steady_clock::now();
//    SendManyFiles(files);
//    auto multi_end = steady_clock::now();
//    auto multi_time = duration_cast<milliseconds>(multi_end - multi_start).count();
//
//    bool multi_ok = true;
//    for (auto it = files.begin(); it != files.end(); ++it) {
//        uint32_t id = it->first;
//        const fileVector& data = it->second;
//        if (collector.GetFileReadOnly(id) != data) {
//            multi_ok = false;
//            break;
//        }
//    }
//
//    ResetCollector();
//    std::cout << "\n=== Single-threaded processing ===" << std::endl;
//    auto single_start = steady_clock::now();
//    for (auto it = files.begin(); it != files.end(); ++it) {
//        uint32_t id = it->first;
//        const fileVector& data = it->second;
//        // std::cout << "[File " << id << "] Start sending..." << std::endl;
//        auto start = steady_clock::now();
//
//        // Прямой вызов SendFile
//        SendFile(data, id);
//
//        auto duration = duration_cast<milliseconds>(steady_clock::now() - start).count();
//        //std::cout << "[File " << id << "] Done in " << duration << " ms" << std::endl;
//    }
//    auto single_end = steady_clock::now();
//    auto single_time = duration_cast<milliseconds>(single_end - single_start).count();
//
//   
//    bool single_ok = true;
//    for (auto it = files.begin(); it != files.end(); ++it) {
//        uint32_t id = it->first;
//        const fileVector& data = it->second;
//        if (collector.GetFileReadOnly(id) != data) {
//            single_ok = false;
//            break;
//        }
//    }
//
//   
//    std::cout << "\n=== Benchmark Results ==="
//        << "\nMulti-threaded: " << multi_time << " ms ("
//        << (multi_ok ? "OK" : "FAILED") << ")"
//        << "\nSingle-threaded: " << single_time << " ms ("
//        << (single_ok ? "OK" : "FAILED") << ")"
//        << "\nSpeed ratio: " << (single_time > 0 ? static_cast<double>(single_time) / multi_time : 0)
//        << std::endl;
//}
//
//void Sender::Benchmark(uint32_t fileId, const std::vector<uint8_t>& file, uint32_t chunkNum) {
//    using namespace std::chrono;
//
//    // Многопоточная обработка
//    collector.RemoveFile(fileId);
//
//    std::vector<std::pair<size_t, Chunk>> chunks_ = SplitIntoChunks(file, chunkNum, true);
//    std::vector<std::pair<size_t, Chunk>> chunks = chunks_;
//    auto startMulti = steady_clock::now();
//    collector.CollectFile(fileId, file.size());
//
//    std::vector<std::future<void>> futures;
//    for (auto& pair : chunks) {
//        auto offset = pair.first;
//        auto& chunk = pair.second;
//        futures.emplace_back(std::async(std::launch::async, [this, fileId, offset, chunk = std::move(chunk)]() mutable {
//            collector.OnNewChunk(fileId, offset, chunk);
//            }));
//    }
//    for (auto& f : futures) f.get();
//    auto endMulti = steady_clock::now();
//    auto multiTime = duration_cast<microseconds>(endMulti - startMulti).count();
//
//    bool multiCorrect = (collector.GetFileReadOnly(fileId) == file);
//    collector.RemoveFile(fileId);
//
//    // Однопоточная обработка
//    chunks = chunks_;
//    auto startSingle = steady_clock::now();
//    collector.CollectFile(fileId, file.size());
//    for (auto& pair : chunks) {
//        collector.OnNewChunk(fileId, pair.first, pair.second);
//    }
//    auto endSingle = steady_clock::now();
//    auto singleTime = duration_cast<microseconds>(endSingle - startSingle).count();
//
//    bool singleCorrect = (collector.GetFileReadOnly(fileId) == file);
//    collector.RemoveFile(fileId);
//
//    std::lock_guard<std::mutex> lock(coutMx);
//    std::cout << "Benchmark for file " << fileId << ":\n";
//    std::cout << "Multi-threaded: " << multiTime << " ms, Correct: " << multiCorrect << "\n";
//    std::cout << "Single-threaded: " << singleTime << " ms, Correct: " << singleCorrect << "\n";
//    std::cout << "Single:Multi = " << (double)multiTime / (double)singleTime << std::endl;
//}
