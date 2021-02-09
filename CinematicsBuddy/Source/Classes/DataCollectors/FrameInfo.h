#pragma once
#include "FrameInfoCollectors.h"

class FrameInfo
{
	std::string ReplayID;
	TimeInfo TimeData;
	CameraInfo CameraData;
	BallInfo BallData;
	std::vector<CarInfo> CarData;
	std::vector<CarSeen> CarsSeenData;

public:
    static FrameInfo Get();
    std::string Print(const TimeInfo& FirstFrame, const std::vector<CarSeen>& AllCarsSeen) const;

    const TimeInfo& GetTimeInfoForTest() { return TimeData; }
    const std::vector<CarSeen>& GetCarsSeenForTest() { return CarsSeenData; }
};
