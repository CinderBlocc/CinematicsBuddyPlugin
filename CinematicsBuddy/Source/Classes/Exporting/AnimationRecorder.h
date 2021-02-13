#pragma once
#include "DataCollectors/FrameInfo.h"
#include <string>
#include <deque>
#include <vector>
#include <fstream>

struct ReplayMetadata
{
    std::string ReplayName;
    std::string ReplayID;
    std::string ReplayDate;
    std::string ReplayFPS;
    std::string ReplayFrames;
};

class AnimationRecorder
{
public:
    using RecordingType = std::deque<FrameInfo>;
    using RecordingParam = const RecordingType&;
    using CarsSeenParam = const std::vector<CarSeen>&;
    using StringParam = const std::string&;

    virtual void StartRecording(StringParam InPathName = "", StringParam InFileName = "", StringParam InCameraName = "");
    virtual void StopRecording();
    
    virtual void AddData(const FrameInfo& FrameData);

    bool GetbIsRecording() { return bIsRecording; }
    void SetMaxRecordingTime(float NewTime) { MaxRecordingTime = NewTime; }
    void SetbIncrementFiles(bool bNewValue) { bIncrementFileNames = bNewValue; }

protected:
    bool bIsRecording;
    float MaxRecordingTime;
    bool bIncrementFileNames;
    RecordingType RecordedData;

    bool WriteFile(StringParam InPathName, StringParam InFileName, StringParam InCameraName);
    void WriteFileThread(std::ofstream& FileStream, StringParam InCameraName, RecordingParam TheRecording);
    void WriteHeader(std::ofstream& FileStream, const ReplayMetadata& ReplayInfo, StringParam InCameraName, RecordingParam TheRecording, CarsSeenParam CarsSeenInRecording);
    void WriteRecordedDataToFile(std::ofstream& FileStream, RecordingParam TheRecording, CarsSeenParam CarsSeenInRecording);
    ReplayMetadata GetReplayMetadata();
    std::vector<CarSeen> GetCarsSeenInRecording(RecordingParam TheRecording);
    float GetRecordingDuration(RecordingParam TheRecording);
    float GetAverageFPS(RecordingParam TheRecording);
    bool GetbWasWholeRecordingInSameReplay(RecordingParam TheRecording);
};
