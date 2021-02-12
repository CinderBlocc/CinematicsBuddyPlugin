#pragma once
#include "DataCollectors/FrameInfo.h"
#include <string>
#include <deque>
#include <vector>
#include <fstream>

class AnimationRecorder
{
public:
    virtual void StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName);
    virtual void StopRecording();
    
    virtual void AddData(const FrameInfo& FrameData);

    bool WriteHeader(std::ofstream& FileStream, const std::string& InCameraName);
    float GetAverageFPS();
    bool GetbWasWholeRecordingInSameReplay();

    bool GetbIsRecording() { return bIsRecording; }
    void SetMaxRecordingTime(float NewTime) { MaxRecordingTime = NewTime; }

protected:
    bool bIsRecording;
    float MaxRecordingTime;
    std::deque<FrameInfo> RecordedData;

    std::vector<CarSeen> CarsSeenInHeader;
    std::vector<CarSeen> GetCarsSeenInRecording();
};
