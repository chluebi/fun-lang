#ifndef SOURCE_LOCATION_HPP
#define SOURCE_LOCATION_HPP

#include <string>
#include <iostream>

struct SourceLocation {
    size_t StartPos;
    size_t EndPos;

    SourceLocation(size_t start, size_t end) : StartPos(start), EndPos(end) {}
};

SourceLocation mergeLocations(const SourceLocation& loc1, const SourceLocation& loc2);

#endif