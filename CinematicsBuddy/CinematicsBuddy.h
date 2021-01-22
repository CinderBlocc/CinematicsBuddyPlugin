#pragma once
#pragma comment(lib, "PluginSDK.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <chrono>


class CinematicsBuddy : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	//Cvars
	std::shared_ptr<std::string> cvarFileName;
	std::shared_ptr<std::string> cvarCameraName;
	std::shared_ptr<float> cvarBufferSize;
	std::shared_ptr<std::string> cvarImportFileName;
	std::shared_ptr<bool> useCamVelocity;
	std::shared_ptr<float> cvarCamSpeed;
	std::shared_ptr<float> cvarCamRotationSpeed;
	std::shared_ptr<bool> cvarShowVersionInfo;

	//Structs
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
	};

	//Vectors
	std::vector<FrameInfo> recordingFrames;
	std::vector<FrameInfo> bufferFrames;
	std::vector<std::string> warnings;

public:
	void onLoad() override;
	void onUnload() override;
	
	//Input override
	//void PlayerInputTick(ActorWrapper camInput, void * params, string funcName);
	void PlayerInputTick();

	//Recording
	void RecordStart();
	void RecordStop();
	void BufferStart();
	void BufferCapture();
	void BufferCancel();
	void RecordingFunction();

	//Importing
	void CamPathImport();
	void CamPathApply();
	void CamPathClear();

	//File writing
	std::string FormatFrameData(int index, FrameInfo firstFrame, FrameInfo currentFrame, std::vector<CarsSeen> carsList);
	std::string GetBufferFilenameTime();
	void WriteToFile(std::vector<FrameInfo> data, std::string filename);

	//Utility
	//Quat RotatorToQuat(Rotator convertToQuat);
	ServerWrapper GetCurrentGameState();
	void Render(CanvasWrapper canvas);
    std::string PrintFloat(const float InFloat, const int InDecimals);
    std::string PrintVector(const Vector& InVector, const int InDecimals);
    std::string PrintQuat(const Quat& InQuat, const int InDecimals);
};

