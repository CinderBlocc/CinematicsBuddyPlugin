#pragma once
#include "AnimationRecorder.h"

class AnimationBuffer : public AnimationRecorder
{
public:
    AnimationBuffer();

    void StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName) override;
    void StopRecording() override;
    void CaptureBuffer(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName);

    void AddData(const FrameInfo& FrameData) override;

private:
    void CleanOutdatedData();
    bool IsBufferFrontOutdated();
};
