#pragma once
#include <string>
#include <vector>
#include <deque>

class FrameInfo;

class AnimationBuffer
{
public:
    void StartBuffer();
    void StopBuffer();
    void CaptureBuffer(const std::string& InPathName);

    bool GetbIsRecording() { return bBufferIsActive; }
    void SetMaxBufferLength(float NewLength) { MaxBufferLength = NewLength; }

    void AddData(const FrameInfo& FrameData);
    void CleanOutdatedData();

private:
    bool bBufferIsActive = false;
    float MaxBufferLength = 30.f;
    std::deque<class FrameInfo> BufferData;

    bool IsBufferFrontOutdated();
    std::vector<class CarSeen> GetCarsSeenInBuffer();
};
