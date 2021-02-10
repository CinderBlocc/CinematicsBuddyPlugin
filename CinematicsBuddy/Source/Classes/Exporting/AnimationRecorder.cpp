#include "AnimationRecorder.h"

void AnimationRecorder::StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName)
{
    //Stub function
}

void AnimationRecorder::StopRecording()
{
    //Stub function
}

void AnimationRecorder::AddData(const FrameInfo& FrameData)
{
    if(bIsRecording)
    {
        RecordedData.push_back(FrameData);
    }
}

void AnimationRecorder::WriteHeader(std::ofstream& FileStream, const FileHeaderInfo& HeaderInfo)
{
    if(FileStream.is_open())
    {
        FileStream << HeaderInfo.Print() << '\n';
    }
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
