#include "AnimationRecorder.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "SimpleJSON/json.hpp"
#include <chrono>

void AnimationRecorder::StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName)
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

bool AnimationRecorder::WriteFile(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName)
{
    //Open the file
    std::filesystem::path OutputFilePath = CBUtils::GetExportPathFromString(InPathName, true);
    if(!std::filesystem::exists(OutputFilePath))
    {
        GlobalCvarManager->log("ERROR: Cannot save file. File path (" + OutputFilePath.string() + ") is invalid.");
        return false;
    }
    OutputFilePath += InFileName + EXTENSION_NAME;
    std::ofstream OutputFile(OutputFilePath);

    //Write to the file
    if(OutputFile.is_open())
    {
        std::vector<CarSeen> CarsSeenInRecording = GetCarsSeenInRecording();
        if(!WriteHeader(OutputFile, InCameraName, CarsSeenInRecording))
        {
            GlobalCvarManager->log("ERROR: File saving failed while writing header");
            OutputFile.close();
            return false;
        }
        if(!WriteRecordedDataToFile(OutputFile, CarsSeenInRecording))
        {
            GlobalCvarManager->log("ERROR: File saving failed while writing frame data");
            OutputFile.close();
            return false;
        }
    }
    else
    {
        GlobalCvarManager->log("ERROR: Could not open output file");
        OutputFile.close();
        return false;
    }

    OutputFile.close();
    GlobalCvarManager->log("Successfully wrote file " + OutputFilePath.string());
    return true;
}

bool AnimationRecorder::WriteHeader(std::ofstream& FileStream, const std::string& InCameraName, const std::vector<CarSeen>& CarsSeenInRecording)
{
    //Write recording metadata
    FileStream << "RECORDING METADATA" << '\n';
    FileStream << "Version: " << PLUGIN_VERSION << '\n';
    FileStream << "Camera: " << InCameraName << '\n';
    FileStream << "Average FPS: " << CBUtils::PrintFloat(GetAverageFPS()) << '\n';
    FileStream << "Frames: " << RecordedData.size() << '\n';
    FileStream << "Duration: " << (RecordedData.size() > 1 ? RecordedData.back().GetTimeInfo().GetTimeDifference(RecordedData[0].GetTimeInfo()) : 0.f) << '\n';
    FileStream << std::endl;

    //Write replay metadata if it exists and if currently in replay
    if(GetbWasWholeRecordingInSameReplay() &&
       GlobalGameWrapper->IsInReplay() &&
       !GlobalGameWrapper->GetGameEventAsReplay().IsNull() &&
       GlobalGameWrapper->GetGameEventAsReplay().GetReplay().memory_address != NULL)
    {
        ReplayWrapper Replay = GlobalGameWrapper->GetGameEventAsReplay().GetReplay();
        FileStream << "REPLAY METADATA" << '\n';
        FileStream << "Name: "   << (Replay.GetReplayName().IsNull() ? "" : Replay.GetReplayName().ToString()) << '\n';
        FileStream << "ID: "     << (Replay.GetId().IsNull()         ? "" : Replay.GetId().ToString()) << '\n';
        FileStream << "Date: "   << (Replay.GetDate().IsNull()       ? "" : Replay.GetDate().ToString()) << '\n';
        FileStream << "FPS: "    << CBUtils::PrintFloat(Replay.GetRecordFPS()) << '\n';
        FileStream << "Frames: " << Replay.GetNumFrames() << '\n';
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

    return true;
}

bool AnimationRecorder::WriteRecordedDataToFile(std::ofstream& FileStream, const std::vector<CarSeen>& CarsSeenInRecording)
{
    if(RecordedData.empty())
    {
        GlobalCvarManager->log("ERROR: No recorded data.");
        return false;
    }

    const FrameInfo& FirstFrame = RecordedData[0];
    int FrameIndex = 0;
    for(const auto& DataPoint : RecordedData)
    {
        FileStream << DataPoint.Print(FirstFrame.GetTimeInfo(), FrameIndex, CarsSeenInRecording) << '\n';
        ++FrameIndex;
    }

    return true;
}

float AnimationRecorder::GetAverageFPS()
{
    using namespace std::chrono;

    //Need at least 2 data points to get a time difference
    if(RecordedData.size() < 2)
    {
        return 0.f;
    }

    //Add up all the time differences between each frame
    float TimeDifferencesTotal = 0.f;
    for(size_t i = 1; i < RecordedData.size(); ++i)
    {
        auto TimeDifference = RecordedData[i].GetTimeInfo().TimeCaptured - RecordedData[i-1].GetTimeInfo().TimeCaptured;
        TimeDifferencesTotal += duration_cast<duration<float>>(TimeDifference).count();
    }

    //Get the average
    float AverageDifference = TimeDifferencesTotal / static_cast<float>(RecordedData.size());
    return 1.f / AverageDifference;
}

bool AnimationRecorder::GetbWasWholeRecordingInSameReplay()
{
    //No data. Definitely not in a replay
    if(RecordedData.empty())
    {
        return false;
    }

    //Get the replay ID of the first frame. If it is empty, it was not in a replay
    std::string ReplayID = RecordedData[0].GetReplayID();
    if(ReplayID.empty())
    {
        return false;
    }

    //Check if every frame's ID matches the first frame
    for(const auto& ThisFrame : RecordedData)
    {
        if(ThisFrame.GetReplayID() != ReplayID)
        {
            return false;
        }
    }

    return true;
}

std::vector<CarSeen> AnimationRecorder::GetCarsSeenInRecording()
{
    std::vector<CarSeen> CarsSeenInRecording;

    //Loop through each frame in the recorded data
    for(const auto& DataPoint : RecordedData)
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
