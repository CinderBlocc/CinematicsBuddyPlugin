#pragma once
#pragma comment(lib, "PluginSDK.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"

class CinematicsBuddy : public BakkesMod::Plugin::BakkesModPlugin
{
private:
    //Class pointers
    std::shared_ptr<class AnimationImporter> Importer;
    std::shared_ptr<class AnimationExporter> Exporter;
    std::shared_ptr<class AnimationBuffer>   Buffer;
    std::shared_ptr<class CameraManager>     Camera;
    std::shared_ptr<class UIManager>         UI;
    std::shared_ptr<class UnitConverter>     Converter;

public:
	void onLoad() override;
	void onUnload() override;

	//Tick functions
    void OnViewportTick();
    bool IsValidRecordingMode();
	void RecordingFunction();
};
