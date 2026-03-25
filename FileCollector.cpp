#include "FileCollector.h"

std::mutex coutMutex;

void FileCollector::CollectFile(uint32_t fileId, size_t fileSize) {

	File newFile(fileId, fileSize); 
	std::lock_guard<std::mutex> idToFileLock(idToFileMutex);
	if (idToFile.count(fileId)) {
		std::lock_guard<std::mutex> coutLock (coutMutex);
		std::cout << "File with id: " << fileId << " already exists" << std::endl;
		return;
	}

	this->idToFile[fileId] = std::move(newFile); 
	this->fileMutexes[fileId];
}


// Будем принимать чанк по ссылке, так как хотим только скопировать его часть.
// Сложность такого алгоритма сборки файла O(N). 
void FileCollector::OnNewChunk(uint32_t fileId, size_t pos, Chunk& chunk) {

	File* filePtr = nullptr;
	{
		std::lock_guard<std::mutex> lock(idToFileMutex);
		auto it = idToFile.find(fileId);
		if (it == idToFile.end()) {
			std::lock_guard<std::mutex> coutLock(coutMutex);
			std::cout << "No such file with Id:" << fileId << std::endl;
			return;
		}
		filePtr = &it->second;
	}

	std::lock_guard<std::mutex> fileLock(fileMutexes[fileId]);
	auto& coverMap = filePtr->GetCoverMap();
	auto& buffer = filePtr->GetFile();


	auto uncovered = coverMap.AddAndGetUncovered(pos, pos + chunk.size());
	
	for (const auto& interval : uncovered) {
		auto start = interval.first;
		auto end = interval.second;
		size_t offset = start - pos;
		std::copy(chunk.begin() + offset, chunk.begin() + offset + (end - start),
			buffer.begin() + start);
	}
}


std::vector<uint8_t>& FileCollector::GetFile(uint32_t fileId) {
	std::lock_guard<std::mutex> lock(idToFileMutex);
	return idToFile[fileId].GetFile();
}

const std::vector<uint8_t>& FileCollector::GetFileReadOnly(uint32_t fileId){
	std::lock_guard<std::mutex> lock(idToFileMutex);
	return idToFile[fileId].GetFile();
	
}

void FileCollector::Reset() {
	std::lock_guard<std::mutex> lock(idToFileMutex);
	idToFile.clear();
	fileMutexes.clear();
}

void FileCollector::RemoveFile(uint32_t fileId) {
	std::lock_guard<std::mutex> lock(idToFileMutex);
	if (idToFile.find(fileId) != idToFile.end()) {
		idToFile.erase(fileId);
		fileMutexes.erase(fileId);
	}
}

