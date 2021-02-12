#pragma once
#include "AnimationRecorder.h"
#include <filesystem>
#include <fstream>
#include <chrono>

class AnimationExporter : public AnimationRecorder
{
public:
    AnimationExporter();

    void StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName) override;
    void StopRecording() override;
    
    void AddData(const FrameInfo& FrameData) override;

private:
    int FramesInTempFile;
    std::ofstream TempFile;
    std::filesystem::path GetTempExportFilePath();

    std::string PendingPathName;
    std::string PendingFileName;
    std::string PendingCameraName;
};
