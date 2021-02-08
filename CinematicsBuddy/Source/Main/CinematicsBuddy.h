#pragma once
#pragma comment(lib, "PluginSDK.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"

class CinematicsBuddy : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	//Cvars
    std::shared_ptr<std::string> ExportSpecialFilePath;
	std::shared_ptr<std::string> ExportFileName;
	std::shared_ptr<std::string> ExportCameraName;
	std::shared_ptr<std::string> ImportFileName;
	std::shared_ptr<float> BufferSize;
	std::shared_ptr<float> CamSpeed;
	std::shared_ptr<float> CamRotationSpeed;
	std::shared_ptr<bool> bShowVersionInfo;	
	std::shared_ptr<bool> bUseCamVelocity;

	//Vectors
	std::vector<FrameInfo> recordingFrames;
	std::vector<FrameInfo> bufferFrames;
	std::vector<std::string> WarningStrings;

    std::shared_ptr<class AnimationImporter> Importer;
    std::shared_ptr<class AnimationExporter> Exporter;

    bool bRecording = false;
    bool bBufferIsActive = false;

public:
	void onLoad() override;
	void onUnload() override;
    void GenerateSettingsFile();
	
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
	void WriteToFile(std::vector<FrameInfo> data, std::string filename);

	//Utility
	//Quat RotatorToQuat(Rotator convertToQuat);
	void Render(CanvasWrapper canvas);
};

