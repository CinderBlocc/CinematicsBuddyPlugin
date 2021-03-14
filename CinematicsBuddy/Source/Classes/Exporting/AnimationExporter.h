#pragma once
#include "AnimationRecorder.h"
#include <filesystem>
#include <chrono>

class AnimationExporter : public AnimationRecorder
{
public:
    AnimationExporter();

    void AddData(const FrameInfo& FrameData) override;

private:
	std::shared_ptr<bool>  bIsRecordingActive = std::make_shared<bool>(false);
    std::shared_ptr<float> RecordSize         = std::make_shared<float>(300.f);
    void OnMaxRecordingTimeChanged() override;

    void StartRecording() override;
    void StopRecording() override;
    void OnNewMapLoading();

    int FramesInTempFile = 0;
    std::ofstream TempFile;
    std::filesystem::path GetTempExportFilePath();

    std::string PendingPathName;
    std::string PendingFileName;
    std::string PendingCameraName;
};
