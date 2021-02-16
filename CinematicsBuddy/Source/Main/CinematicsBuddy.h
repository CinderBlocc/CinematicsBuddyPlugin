#pragma once
#pragma comment(lib, "PluginSDK.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"

class CinematicsBuddy : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	//Mutual recording cvars
    std::shared_ptr<bool>  bIsFileWriting;
	std::shared_ptr<bool>  bIncrementFileNames;
	std::shared_ptr<bool>  bSetSpecialFilePath;
    std::shared_ptr<std::string> ExportSpecialFilePath;
	std::shared_ptr<std::string> ExportFileName;
	std::shared_ptr<std::string> ExportCameraName;

    //Exporting cvars
	std::shared_ptr<float> RecordSize;
	std::shared_ptr<bool>  bIsRecordingActive;

    //Buffer cvars
	std::shared_ptr<float> BufferSize;
	std::shared_ptr<bool>  bIsBufferActive;
	

    //Class pointers
    std::shared_ptr<class AnimationImporter> Importer;
    std::shared_ptr<class AnimationExporter> Exporter;
    std::shared_ptr<class AnimationBuffer>   Buffer;
    std::shared_ptr<class CameraManager>     Camera;

public:
	void onLoad() override;
	void onUnload() override;

    //Utility
    void GenerateSettingsFile();
    std::string GetBindingsList();
    std::string GetSpecialFilePath();
    void OnNewMapLoading();

    //Cvar changes
    void OnRecordingSettingChanged(ERecordingSettingChanged ChangedValue);

	//Recording
    bool IsValidRecordingMode();
	void RecordingFunction();
	void RecordStart();
	void RecordStop();
	void BufferCapture();
    void BufferClear();

    //Input override
    bool IsValidCamOverrideMode();
	void PlayerInputTick();

	//Importing
	void CamPathImport();
	void CamPathApply();
	void CamPathClear();

    // TESTING - REMOVE WHEN DONE //
    void TestExportFormat();
    void TestPrintFloat();
    void TestInputs();
    void DebugRender(CanvasWrapper Canvas);
};

