#pragma once
#pragma comment(lib, "PluginSDK.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"

class CinematicsBuddy : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	//Cvars
    std::shared_ptr<std::string> ExportSpecialFilePath;
	std::shared_ptr<std::string> ExportFileName;
	std::shared_ptr<std::string> ExportCameraName;
	std::shared_ptr<std::string> ImportFileName;
	std::shared_ptr<float> RecordSize;
	std::shared_ptr<float> BufferSize;
	std::shared_ptr<float> CamSpeed;
	std::shared_ptr<float> CamRotationSpeed;
	std::shared_ptr<bool> bSetSpecialFilePath;
	std::shared_ptr<bool> bIncrementFileNames;
	std::shared_ptr<bool> bUseCamOverrides;
	std::shared_ptr<bool> bIsRecordingActive;
	std::shared_ptr<bool> bIsBufferActive;

	//Vectors
	std::vector<std::string> WarningStrings;

    std::shared_ptr<class AnimationImporter> Importer;
    std::shared_ptr<class AnimationExporter> Exporter;
    std::shared_ptr<class AnimationBuffer>   Buffer;
    std::shared_ptr<class CameraManager>     Camera;

    bool bRecording = false;
    bool bBufferIsActive = false;

public:
	void onLoad() override;
	void onUnload() override;

    //Utility
    bool IsValidRecordingMode();
    bool IsValidCamOverrideMode();
    void GenerateSettingsFile();
    std::string GetSpecialFilePath();
    void OnNewMapLoading();
    
    // TESTING - REMOVE WHEN DONE //
    void TestExportFormat();
    void TestPrintFloat();
    void DebugRender(CanvasWrapper Canvas);

	//Recording
	void RecordingFunction();
	void RecordStart();
	void RecordStop();
	void BufferCapture();
    void BufferClear();

    //Cvar changes
    void OnIncrementChanged();
    void OnBufferEnabledChanged();
    void OnMaxBufferLengthChanged();
    void OnMaxRecordingLengthChanged();
    void OnCamOverridesChanged();

    //Input override
	void PlayerInputTick();

	//Importing
	void CamPathImport();
	void CamPathApply();
	void CamPathClear();
};

