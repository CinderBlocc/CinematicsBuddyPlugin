#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <chrono>

namespace json
{
    class JSON;
}

struct TimeInfo
{
	std::chrono::steady_clock::time_point TimeCaptured;
	int ReplayFrame;

    static TimeInfo Get(ReplayServerWrapper Replay);
    float GetTimeDifference(const TimeInfo& FirstFrame) const;
    json::JSON ConvertToJSON(const TimeInfo& FirstFrame) const;
};

struct CameraInfo
{
    bool bIsNull;
	Vector Location;
	Quat Rotation;
	float FOV;

    static CameraInfo Get(CameraWrapper Camera);
    json::JSON ConvertToJSON() const;
};

struct BallInfo
{
    bool bIsNull;
	Vector Location;
	Quat Rotation;

    static BallInfo Get(ServerWrapper Server);
    json::JSON ConvertToJSON() const;
};

struct WheelInfo
{
    bool bIsNull;
	float SteerAmount;
	float SuspensionDistance;
	float SpinSpeed;
	//float Radius; //Stored in CarsSeen so it only needs to be written to file once

    static WheelInfo Get(WheelWrapper Wheel);
    json::JSON ConvertToJSON(int WheelIndex) const;
};

struct CarInfo
{
    bool bIsNull;
	UniqueIDWrapper ID;
	bool bIsBoosting;
	Vector Location;
	Quat Rotation;
	WheelInfo Wheels[4];

    static CarInfo Get(CarWrapper Car);
    std::string GetCarSeenIndex(const std::vector<struct CarSeen>& AllCarsSeen) const;
    json::JSON ConvertToJSON() const;
};

struct CarSeen
{
    bool bIsNull;
	UniqueIDWrapper ID;
	int Body;
	float FrontWheelRadius;
	float BackWheelRadius;

    static CarSeen Get(CarWrapper Car);
};
inline bool operator==(const CarSeen c1, const CarSeen c2)
{
    return c1.ID.GetIdString() == c2.ID.GetIdString();
}
