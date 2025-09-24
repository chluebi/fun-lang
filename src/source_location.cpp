#include <algorithm> 

#include "source_location.hpp"

SourceLocation mergeLocations(const SourceLocation& loc1, const SourceLocation& loc2) {
    size_t newStart = std::min(loc1.StartPos, loc2.StartPos);
    size_t newEnd = std::max(loc1.EndPos, loc2.EndPos);
    return SourceLocation(newStart, newEnd);
}

