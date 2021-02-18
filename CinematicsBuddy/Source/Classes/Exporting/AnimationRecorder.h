#pragma once
#include "DataCollectors/FrameInfo.h"
#include <string>
#include <deque>
#include <vector>
#include <fstream>

class UIManager;

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

    AnimationRecorder(std::shared_ptr<UIManager> TheUI);
    
    bool GetbIsRecording() { return bIsRecording; }
    virtual void AddData(const FrameInfo& FrameData);

protected:
    std::shared_ptr<UIManager> UI;

    bool bIncrementFileNames = true;
    bool bIsRecording        = false;
    float MaxRecordingTime   = 0.f;
    RecordingType RecordedData;
    bool HaveCvarsBeenInitialzed();
    void OnIncrementFilesChanged();
    virtual void OnMaxRecordingTimeChanged();

    virtual void StartRecording();
    virtual void StopRecording();

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
