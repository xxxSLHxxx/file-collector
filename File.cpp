#include"File.h"

File::File(uint32_t fileId, size_t size): coverMap(IntervalCoverMap()), file(std::vector<uint8_t>(size)), fileId(fileId) { }

File::File() : coverMap(IntervalCoverMap()), file(std::vector<uint8_t>()), fileId(0) { }
