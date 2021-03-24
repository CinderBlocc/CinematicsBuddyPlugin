#pragma once
#include "AnimationRecorder.h"

class UIManager;

class AnimationBuffer : public AnimationRecorder
{
public:
    AnimationBuffer(std::shared_ptr<UIManager> TheUI);

    void StartRecording() override;
    void StopRecording() override;
    void CaptureBuffer();
    void ClearBuffer();

    void AddData(const FrameInfo& FrameData) override;

private:
	std::shared_ptr<bool>  bIsBufferActive = std::make_shared<bool>(false);
    std::shared_ptr<float> BufferSize      = std::make_shared<float>(30.f);

    void CleanOutdatedData();
    bool IsBufferFrontOutdated();

    void OnBufferEnabledChanged();
    void OnMaxRecordingTimeChanged() override;
};
