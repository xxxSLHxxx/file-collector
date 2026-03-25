#pragma once
#include<vector>
#include<map>
#include <utility>
class IntervalCoverMap {
public:
    // Добавляет новый интервал [start, end) и возвращает вектор непокрытых подотрезков
    std::vector<std::pair<size_t, size_t>> AddAndGetUncovered(size_t start, size_t end);

private:
    // std::map: key — левая граница интервала, value — правая
    std::map<size_t, size_t> covered;
};
