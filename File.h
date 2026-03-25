#pragma once
#include<mutex>
#include<vector>
#include"IntervalCoverMap.h"

class File {
public:
	
	File(uint32_t fileId, size_t size);
	File();
	std::vector<uint8_t>& GetFile() { return file; }
	IntervalCoverMap& GetCoverMap() { return coverMap; }
	size_t Size() const { return file.size(); }


private: 

	IntervalCoverMap coverMap;
	std::vector<uint8_t> file;
	uint32_t fileId;

};