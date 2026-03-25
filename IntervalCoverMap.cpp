#include"IntervalCoverMap.h"
#include<vector>

std::vector<std::pair<size_t, size_t>> IntervalCoverMap::AddAndGetUncovered(size_t start, size_t end) {
        std::vector<std::pair<size_t, size_t>> uncovered;

        if (start >= end) return uncovered;

        auto it = covered.lower_bound(start);
        if (it != covered.begin()) --it;

        std::vector<std::map<size_t, size_t>::iterator> toErase;

        size_t newStart = start;
        size_t newEnd = end;

        while (it != covered.end() && it->first < end) {
            size_t curStart = it->first;
            size_t curEnd = it->second;

            if (curEnd < start) {
                ++it;
                continue;
            }

            newStart = std::min(newStart, curStart);
            newEnd = std::max(newEnd, curEnd);

            if (start < curStart) {
                uncovered.emplace_back(start, std::min(curStart, end));
            }

            start = std::max(start, curEnd);

            toErase.push_back(it);
            ++it;
        }

        if (start < end) {
            uncovered.emplace_back(start, end);
        }

        for (auto& e : toErase) covered.erase(e);
        covered[newStart] = newEnd;

        return uncovered;
    }


