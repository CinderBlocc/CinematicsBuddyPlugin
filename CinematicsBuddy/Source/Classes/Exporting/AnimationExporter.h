#pragma once
#include "AnimationRecorder.h"
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
    std::ofstream TempFile;

    //Store the values from StartRecording to use when creating the final file
    std::string PendingPathName;
    std::string PendingFileName;
    std::string PendingCameraName;
};
