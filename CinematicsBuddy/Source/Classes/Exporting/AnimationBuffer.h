#pragma once
#include "AnimationRecorder.h"

class AnimationBuffer : public AnimationRecorder
{
public:
    AnimationBuffer();

    void StartRecording(StringParam InPathName = "", StringParam InFileName = "", StringParam InCameraName = "") override;
    void StopRecording() override;
    void CaptureBuffer(StringParam InPathName, StringParam InFileName, StringParam InCameraName);

    void AddData(const FrameInfo& FrameData) override;

private:
    void CleanOutdatedData();
    bool IsBufferFrontOutdated();
};
