#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <chrono>


extern std::shared_ptr<CVarManagerWrapper> GlobalCvarManager;
extern std::shared_ptr<GameWrapper>        GlobalGameWrapper;


// MACROS //
#define PLUGIN_VERSION "0.9.7"
#define EXTENSION_NAME ".txt"


// STRUCTS //
struct TimeInfo
{
	std::chrono::steady_clock::time_point timeCaptured;
	int replayFrame;
};

struct CameraInfo
{
	Vector location;
	Quat orientation;
	float FOV = 90;
};

struct BallInfo
{
	Vector location;
	Quat orientation;
};

struct WheelInfo
{
	float steer;
	//float radius;
	float suspensionDistance;
	float spinSpeed;
};

struct CarInfo
{
	UniqueIDWrapper ID;
	bool isBoosting;
	Vector location;
	Quat orientation;
	WheelInfo wheels[4];
};

struct CarsSeen
{
	std::chrono::steady_clock::time_point timeSeen;
	UniqueIDWrapper ID;
	int body;
	float wheels01Radius;
	float wheels23Radius;
};

struct FrameInfo
{
	std::chrono::steady_clock::time_point timeCaptured;
	std::string replayID = "NULL";
	TimeInfo timeInfo;
	CameraInfo cameraInfo;
	BallInfo ballInfo;
	std::vector<CarInfo> carInfo;
	std::vector<CarsSeen> carsSeenInfo;

    std::string Print() const
    {
        return "";
    }
};

struct FileHeaderInfo
{
    std::string Print() const
    {
        return "";
    }
};


// ENUMS //
