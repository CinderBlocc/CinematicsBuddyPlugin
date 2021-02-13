#include "AnimationRecorder.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "SimpleJSON/json.hpp"
#include "Misc/CBTimer.h"
#include <chrono>

void AnimationRecorder::StartRecording(StringParam InPathName, StringParam InFileName, StringParam InCameraName)
{
    //Stub function
    if(bIsRecording)
    {
        return;
    }
}

void AnimationRecorder::StopRecording()
{
    //Stub function
    if(!bIsRecording)
    {
        return;
    }
}

void AnimationRecorder::AddData(const FrameInfo& FrameData)
{
    if(bIsRecording)
    {
        RecordedData.push_back(FrameData);
    }
}

std::filesystem::path AnimationRecorder::GetFinalFileName(std::filesystem::path IntendedPath, StringParam InFileName)
{
    std::filesystem::path Output = IntendedPath;

    // @TODO: Do something recursive in here or some loopy magic to iterate attempts
    // Recursive might be easiest if you pass in `int NumAttempts`
    // First check should be seeing if this needs to increment in the first place (check if bIncrementFileNames is true)
    // Append _## to the filename. Only up to _99. From there just overwrite _99 each time
    //     NOTE: 1-9 should be _01, _02, etc so sorting doesn't freak out and put _1 next to _10

    Output += InFileName + EXTENSION_NAME;

    return Output;
}

bool AnimationRecorder::WriteFile(StringParam InPathName, StringParam InFileName, StringParam InCameraName)
{
    // @TODO: Launch WriteFileThread in a new thread
    // Mutex a bool bIsWritingFile or something

    //Check if the recording has any data
    if(RecordedData.empty())
    {
        GlobalCvarManager->log("ERROR: Cannot save file. No recorded data.");
        return false;
    }

    //Open the file
    std::filesystem::path OutputFilePath = CBUtils::GetExportPathFromString(InPathName, true);
    if(!std::filesystem::exists(OutputFilePath))
    {
        GlobalCvarManager->log("ERROR: Cannot save file. File path (" + OutputFilePath.string() + ") is invalid.");
        return false;
    }
    OutputFilePath = GetFinalFileName(OutputFilePath, InFileName);
    std::ofstream OutputFile(OutputFilePath);

    //Write to the file
    if(OutputFile.is_open())
    {
        //Create a copy of the data to work from in the async task
        std::deque<FrameInfo> RecordedDataCopy = RecordedData;

        // @TODO: Launch this in a thread
        GlobalCvarManager->getCvar(CVAR_IS_FILE_WRITING).setValue(true);
        WriteFileThread(OutputFile, InCameraName, RecordedDataCopy);
        GlobalCvarManager->getCvar(CVAR_IS_FILE_WRITING).setValue(false);
    }
    else
    {
        GlobalCvarManager->log("ERROR: Could not open output file");
        OutputFile.close();
        return false;
    }

    // @TODO: Log this and return true after waiting for WriteFileThread to finish?
    GlobalCvarManager->log("Successfully wrote file " + OutputFilePath.string());
    return true;
}

void AnimationRecorder::WriteFileThread(std::ofstream& FileStream, StringParam InCameraName, RecordingParam TheRecording)
{
    std::vector<CarSeen> CarsSeenInRecording = GetCarsSeenInRecording(TheRecording);
    WriteHeader(FileStream, GetReplayMetadata(), InCameraName, TheRecording, CarsSeenInRecording);
    WriteRecordedDataToFile(FileStream, TheRecording, CarsSeenInRecording);

    FileStream.close();
}

void AnimationRecorder::WriteHeader(std::ofstream& FileStream, const ReplayMetadata& ReplayInfo, StringParam InCameraName, RecordingParam TheRecording, CarsSeenParam CarsSeenInRecording)
{
    //Write recording metadata
    FileStream << "RECORDING METADATA" << '\n';
    FileStream << "Version: "     << PLUGIN_VERSION << '\n';
    FileStream << "Camera: "      << InCameraName << '\n';
    FileStream << "Average FPS: " << CBUtils::PrintFloat(GetAverageFPS(TheRecording)) << '\n';
    FileStream << "Frames: "      << TheRecording.size() << '\n';
    FileStream << "Duration: "    << CBUtils::PrintFloat(GetRecordingDuration(TheRecording)) << '\n';
    FileStream << std::endl;

    //Write replay metadata if it exists and if currently in replay
    if(GetbWasWholeRecordingInSameReplay(TheRecording))
    {
        FileStream << "REPLAY METADATA" << '\n';
        FileStream << "Name: "   << ReplayInfo.ReplayName   << '\n';
        FileStream << "ID: "     << ReplayInfo.ReplayID     << '\n';
        FileStream << "Date: "   << ReplayInfo.ReplayDate   << '\n';
        FileStream << "FPS: "    << ReplayInfo.ReplayFPS    << '\n';
        FileStream << "Frames: " << ReplayInfo.ReplayFrames << '\n';
        FileStream << std::endl;
    }

    //Write cars seen
    if(!CarsSeenInRecording.empty())
    {
        int CarIndex = 0;
        json::JSON CarsSeenJSON = json::Object();
        for(const auto& Car : CarsSeenInRecording)
        {
            CarsSeenJSON[std::to_string(CarIndex)] = Car.ConvertToJSON();
            ++CarIndex;
        }

        FileStream << "CARS SEEN" << '\n';
        FileStream << CarsSeenJSON.dump(1, "\t") << '\n';
        FileStream << std::endl;
    }

    //Write example format
    FileStream << "EXAMPLE FRAME FORMAT" << '\n';
    FileStream << FrameInfo::PrintExampleFormat() << '\n';
    FileStream << std::endl;

    //Mark the header as complete so parsers know to start looping frames
    FileStream << "BEGIN ANIMATION" << std::endl;
}

void AnimationRecorder::WriteRecordedDataToFile(std::ofstream& FileStream, RecordingParam TheRecording, CarsSeenParam CarsSeenInRecording)
{
    const FrameInfo& FirstFrame = TheRecording[0];
    int FrameIndex = 0;
    for(const auto& DataPoint : TheRecording)
    {
        FileStream << DataPoint.Print(FirstFrame.GetTimeInfo(), FrameIndex, CarsSeenInRecording) << '\n';
        ++FrameIndex;
    }
}

float AnimationRecorder::GetRecordingDuration(RecordingParam TheRecording)
{
    //Need 2 data points to get a duration
    if(TheRecording.size() < 2)
    {
        return 0.f;
    }
    
    const TimeInfo& FirstFrame = TheRecording[0].GetTimeInfo();
    const TimeInfo& LastFrame = TheRecording.back().GetTimeInfo();

    return LastFrame.GetTimeDifference(FirstFrame);
}

float AnimationRecorder::GetAverageFPS(RecordingParam TheRecording)
{
    using namespace std::chrono;

    //Need at least 2 data points to get a time difference
    if(TheRecording.size() < 2)
    {
        return 0.f;
    }

    //Add up all the time differences between each frame
    float TimeDifferencesTotal = 0.f;
    for(size_t i = 1; i < TheRecording.size(); ++i)
    {
        auto TimeDifference = TheRecording[i].GetTimeInfo().TimeCaptured - TheRecording[i-1].GetTimeInfo().TimeCaptured;
        TimeDifferencesTotal += duration_cast<duration<float>>(TimeDifference).count();
    }

    //Get the average
    float AverageDifference = TimeDifferencesTotal / static_cast<float>(TheRecording.size());
    return 1.f / AverageDifference;
}

bool AnimationRecorder::GetbWasWholeRecordingInSameReplay(RecordingParam TheRecording)
{
    //No data. Definitely not in a replay
    if(TheRecording.empty())
    {
        return false;
    }

    //Get the replay ID of the first frame. If it is empty, it was not in a replay
    std::string ReplayID = TheRecording[0].GetReplayID();
    if(ReplayID.empty())
    {
        return false;
    }

    //Check if every frame's ID matches the first frame
    for(const auto& ThisFrame : TheRecording)
    {
        if(ThisFrame.GetReplayID() != ReplayID)
        {
            return false;
        }
    }

    return true;
}

ReplayMetadata AnimationRecorder::GetReplayMetadata()
{
    ReplayMetadata Output;

    if(!GlobalGameWrapper->IsInReplay()) { return Output; }
    ReplayServerWrapper Server = GlobalGameWrapper->GetGameEventAsReplay();
    if(Server.IsNull()) { return Output; }
    ReplayWrapper Replay = Server.GetReplay();
    if(Replay.memory_address == NULL) { return Output; }

    Output.ReplayName   = Replay.GetReplayName().IsNull() ? "" : Replay.GetReplayName().ToString();
    Output.ReplayID     = Replay.GetId().IsNull() ? "" : Replay.GetId().ToString();
    Output.ReplayDate   = Replay.GetDate().IsNull() ? "" : Replay.GetDate().ToString();
    Output.ReplayFPS    = CBUtils::PrintFloat(Replay.GetRecordFPS());
    Output.ReplayFrames = std::to_string(Replay.GetNumFrames());

    return Output;
}

std::vector<CarSeen> AnimationRecorder::GetCarsSeenInRecording(RecordingParam TheRecording)
{
    std::vector<CarSeen> CarsSeenInRecording;

    //Loop through each frame in the recorded data
    for(const auto& DataPoint : TheRecording)
    {
        //Loop through each of the cars this frame
        auto CarsSeenThisFrame = DataPoint.GetCarsSeen();
        for(const auto& CarSeenThisFrame : CarsSeenThisFrame)
        {
            if(CarSeenThisFrame.bIsNull)
            {
                continue;
            }

            //Compare this car to the cars in the already-seen list
            bool bHasSeenCar = false;
            for(const auto& CarSeenInRecording : CarsSeenInRecording)
            {
                if(CarSeenThisFrame == CarSeenInRecording)
                {
                    bHasSeenCar = true;
                    break;
                }
            }

            //If this car isn't in the list, add it
            if(!bHasSeenCar)
            {
                CarsSeenInRecording.push_back(CarSeenThisFrame);
            }
        }
    }

    return CarsSeenInRecording;
}
