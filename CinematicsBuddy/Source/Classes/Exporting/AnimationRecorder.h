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

    std::shared_ptr<bool> bIsFileWriting      = std::make_shared<bool>(false);
    std::shared_ptr<bool> bIncrementFileNames = std::make_shared<bool>(true);
    std::shared_ptr<bool> bSetSpecialPath     = std::make_shared<bool>(false);
    std::shared_ptr<std::string> SpecialPath  = std::make_shared<std::string>("");
    std::shared_ptr<std::string> FileName     = std::make_shared<std::string>("");
    std::shared_ptr<std::string> CameraName   = std::make_shared<std::string>("");

    bool bIsRecording        = false;
    float MaxRecordingTime   = 0.f;
    RecordingType RecordedData;
    bool HaveCvarsBeenInitialzed();
    virtual void OnMaxRecordingTimeChanged();

    virtual void StartRecording();
    virtual void StopRecording();

    bool WriteFile(StringParam InPathName, StringParam InFileName, StringParam InCameraName);
    void WriteFileThread(std::filesystem::path OutputFilePath, StringParam InCameraName, RecordingParam TheRecording);
    void WriteHeader(std::ofstream& FileStream, const ReplayMetadata& ReplayInfo, StringParam InCameraName, RecordingParam TheRecording, CarsSeenParam CarsSeenInRecording);
    void WriteRecordedDataToFile(std::ofstream& FileStream, RecordingParam TheRecording, CarsSeenParam CarsSeenInRecording);
    ReplayMetadata GetReplayMetadata();
    std::vector<CarSeen> GetCarsSeenInRecording(RecordingParam TheRecording);
    float GetRecordingDuration(RecordingParam TheRecording);
    float GetAverageFPS(RecordingParam TheRecording);
    bool GetbWasWholeRecordingInSameReplay(RecordingParam TheRecording);
};
