#pragma once
#include "DataCollectors/FrameInfoCollectors.h"

struct FileHeaderInfo
{
    std::vector<CarSeen> CarsSeenInRecording;

    std::string Print() const;
};
