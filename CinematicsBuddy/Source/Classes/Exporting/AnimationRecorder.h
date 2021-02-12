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

    bool GetbIsRecording() { return bIsRecording; }
    void SetMaxRecordingTime(float NewTime) { MaxRecordingTime = NewTime; }

protected:
    bool bIsRecording;
    float MaxRecordingTime;
    std::deque<FrameInfo> RecordedData;

    bool WriteFile(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName);
    bool WriteHeader(std::ofstream& FileStream, const std::string& InCameraName, const std::vector<CarSeen>& CarsSeenInRecording);
    bool WriteRecordedDataToFile(std::ofstream& FileStream, const std::vector<CarSeen>& CarsSeenInRecording);
    std::vector<CarSeen> GetCarsSeenInRecording();
    float GetAverageFPS();
    bool GetbWasWholeRecordingInSameReplay();
};
