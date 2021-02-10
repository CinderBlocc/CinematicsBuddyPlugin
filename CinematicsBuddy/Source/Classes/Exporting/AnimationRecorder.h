#pragma once
#include "DataCollectors/FrameInfo.h"
#include <string>
#include <deque>
#include <vector>
#include <fstream>

class FileHeaderInfo;

class AnimationRecorder
{
public:
    virtual void StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName);
    virtual void StopRecording();
    
    virtual void AddData(const FrameInfo& FrameData);

    std::string WriteHeader(std::ofstream& FileStream, const FileHeaderInfo& HeaderInfo);

    bool GetbIsRecording() { return bIsRecording; }
    void SetMaxRecordingTime(float NewTime) { MaxRecordingTime = NewTime; }

protected:
    bool bIsRecording;
    float MaxRecordingTime;
    std::deque<FrameInfo> RecordedData;

    std::vector<CarSeen> GetCarsSeenInRecording();
};
