#pragma once
#include "FrameInfoCollectors.h"

class FrameInfo
{
public:
    static FrameInfo Get();
    std::string Print(const TimeInfo& FirstFrame, int FrameIndex, const std::vector<CarSeen>& AllCarsSeen) const;
    static std::string PrintExampleFormat();

    const std::string GetReplayID()           const { return ReplayID; }
    const TimeInfo& GetTimeInfo()             const { return TimeData; }
    const CameraInfo& GetCameraInfo()         const { return CameraData; }
    const BallInfo& GetBallInfo()             const { return BallData; }
    const std::vector<CarInfo>& GetCarInfo()  const { return CarData; }
    const std::vector<CarSeen>& GetCarsSeen() const { return CarsSeenData; }

private:
    std::string ReplayID;
	TimeInfo TimeData;
	CameraInfo CameraData;
	BallInfo BallData;
	std::vector<CarInfo> CarData;
	std::vector<CarSeen> CarsSeenData;
};
