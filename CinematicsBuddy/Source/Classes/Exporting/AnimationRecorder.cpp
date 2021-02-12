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

bool AnimationRecorder::WriteHeader(std::ofstream& FileStream, const std::string& InCameraName)
{
    CarsSeenInHeader.clear();
    if(!FileStream.is_open())
    {
        return false;
    }

    //Write recording metadata
    FileStream << "RECORDING METADATA" << '\n';
    FileStream << "Version: " << PLUGIN_VERSION << '\n';
    FileStream << "Camera: " << InCameraName << '\n';
    FileStream << "Average FPS: " << CBUtils::PrintFloat(GetAverageFPS()) << '\n';
    FileStream << "Frames: " << RecordedData.size() << '\n';
    FileStream << std::endl;

    //Write replay metadata if it exists
    if(GetbWasWholeRecordingInSameReplay())
    {
        FileStream << "REPLAY METADATA" << '\n';

        /*
        
            Should replay medatada be captured here? What if recording stops because the game mode is destroyed?
            If you pre hook the game mode destroyed event, does the mode still exist for you to grab the info?

            REPLAY METADATA HERE
            Name
            ID
            Date
            FPS
            Frames

        */

        FileStream << std::endl;
    }

    //Write cars seen
    CarsSeenInHeader = GetCarsSeenInRecording();
    if(!CarsSeenInHeader.empty())
    {
        int CarIndex = 0;
        json::JSON CarsSeenJSON = json::Object();
        for(const auto& Car : CarsSeenInHeader)
        {
            CarsSeenJSON[CarIndex] = Car.ConvertToJSON();
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
    return 1 / AverageDifference;
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
