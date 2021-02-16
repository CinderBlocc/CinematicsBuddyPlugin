#pragma once
#pragma comment(lib, "PluginSDK.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"

class CinematicsBuddy : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	//Recording cvars
    std::shared_ptr<std::string> ExportSpecialFilePath;
	std::shared_ptr<std::string> ExportFileName;
	std::shared_ptr<std::string> ExportCameraName;
	std::shared_ptr<float> RecordSize;
	std::shared_ptr<float> BufferSize;
	std::shared_ptr<bool>  bSetSpecialFilePath;
	std::shared_ptr<bool>  bIncrementFileNames;
	std::shared_ptr<bool>  bIsRecordingActive;
	std::shared_ptr<bool>  bIsBufferActive;
    std::shared_ptr<bool>  bIsFileWriting;

    //Importing cvars
	std::shared_ptr<std::string> ImportFileName;

    //Camera override cvars
    std::shared_ptr<bool>  bUseCamOverrides;
    std::shared_ptr<bool>  bUseLocalMatrix;
    std::shared_ptr<float> CamMovementSpeed;
    std::shared_ptr<float> CamMovementAccel;
    std::shared_ptr<float> CamRotationAccel;
    std::shared_ptr<float> CamMouseSensitivity;
    std::shared_ptr<float> CamGamepadSensitivity;
    std::shared_ptr<float> CamFOVRotationScale;
    std::shared_ptr<std::string> CamRollBinding;
    int CamRollBindingIndex = 0;

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
    void OnCamOverridesChanged(ECamOverrideChanged ChangedValue);

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

