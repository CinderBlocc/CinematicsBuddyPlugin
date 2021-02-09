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
    void AddToJSON(json::JSON& JSONState, const TimeInfo& FirstFrame);
};

struct CameraInfo
{
    bool bIsNull;
	Vector Location;
	Quat Rotation;
	float FOV;

    static CameraInfo Get(CameraWrapper Camera);
    void AddToJSON(json::JSON& JSONState);
};

struct BallInfo
{
    bool bIsNull;
	Vector Location;
	Quat Rotation;

    static BallInfo Get(ServerWrapper Server);
    void AddToJSON(json::JSON& JSONState);
};

struct WheelInfo
{
    bool bIsNull;
	float SteerAmount;
	float SuspensionDistance;
	float SpinSpeed;
	//float Radius; //Stored in CarsSeen so it only needs to be written to file once

    static WheelInfo Get(WheelWrapper Wheel);
    void AddToJSON(json::JSON& JSONState);
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
    void AddToJSON(json::JSON& JSONState, const std::vector<struct CarSeen>& AllCarsSeen);
};

struct CarSeen
{
    bool bIsNull;
	std::chrono::steady_clock::time_point TimeSeen;
	UniqueIDWrapper ID;
	int Body;
	float FrontWheelRadius;
	float BackWheelRadius;

    static CarSeen Get(CarWrapper Car);
};
