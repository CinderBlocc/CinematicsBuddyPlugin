#pragma once
#pragma comment(lib, "BakkesMod.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <chrono>


class CinematicsBuddy : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	//Cvars
	std::shared_ptr<string> cvarFileName;
	std::shared_ptr<string> cvarCameraName;
	std::shared_ptr<float> cvarBufferSize;
	std::shared_ptr<string> cvarImportFileName;
	std::shared_ptr<bool> useCamVelocity;
	std::shared_ptr<float> cvarCamSpeed;
	std::shared_ptr<float> cvarCamRotationSpeed;
	std::shared_ptr<bool> cvarShowVersionInfo;

	//Structs
	struct TimeInfo
	{
		chrono::steady_clock::time_point timeCaptured;
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
		SteamID ID;
		bool isBoosting;
		Vector location;
		Quat orientation;
		WheelInfo wheels[4];
	};
	struct CarsSeen
	{
		chrono::steady_clock::time_point timeSeen;
		SteamID ID;
		int body;
		float wheels01Radius;
		float wheels23Radius;
	};
	struct FrameInfo
	{
		chrono::steady_clock::time_point timeCaptured;
		string replayID = "NULL";
		TimeInfo timeInfo;
		CameraInfo cameraInfo;
		BallInfo ballInfo;
		vector<CarInfo> carInfo;
		vector<CarsSeen> carsSeenInfo;
	};

	//Vectors
	vector<FrameInfo> recordingFrames;
	vector<FrameInfo> bufferFrames;
	vector<string> warnings;

public:
	virtual void onLoad();
	virtual void onUnload();
	
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
	string FormatFrameData(int index, FrameInfo firstFrame, FrameInfo currentFrame, vector<CarsSeen> carsList);
	string GetBufferFilenameTime();
	void WriteToFile(vector<FrameInfo> data, string filename);

	//Utility
	Quat RotatorToQuat(Rotator convertToQuat);
	ServerWrapper GetCurrentGameState();
	void Render(CanvasWrapper canvas);
};

