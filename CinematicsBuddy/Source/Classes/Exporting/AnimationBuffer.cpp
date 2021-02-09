#include "AnimationBuffer.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "DataCollectors/FrameInfo.h"
#include "SupportFiles/CBUtils.h"
#include <fstream>
#include <chrono>

/*

    @TODO:
    - Write buffer header before saving to file

*/

void AnimationBuffer::StartBuffer()
{
    if(bBufferIsActive)
    {
        return;
    }

    BufferData.clear();
    bBufferIsActive = true;
}

void AnimationBuffer::StopBuffer()
{
    bBufferIsActive = false;
    BufferData.clear();
}

void AnimationBuffer::CaptureBuffer(const std::string& InPathName)
{
    if(BufferData.empty())
    {
        return;
    }

    //Open the file
    std::filesystem::path OutputFilePath = CBUtils::GetExportPathFromString(InPathName);
    OutputFilePath += "Buffer_" + CBUtils::GetCurrentTimeAsString() + EXTENSION_NAME;
    std::ofstream BufferFile(OutputFilePath);

    //Write to the file
    if(BufferFile.is_open())
    {
        //Create file header
        FileHeaderInfo FileHeader;

        std::vector<CarSeen> CarsSeenInBuffer = GetCarsSeenInBuffer();

        /*
        
            FILL FILE HEADER INFO HERE
        
        */

        BufferFile << FileHeader.Print() << '\n';

        //Write the buffer data to the file
        const FrameInfo& FirstFrame = BufferData[0];
        for(const auto& DataPoint : BufferData)
        {
            BufferFile << DataPoint.Print(FirstFrame.GetTimeInfo(), CarsSeenInBuffer) << '\n';
        }
        BufferFile << "END" << std::endl;
    }

    BufferFile.close();
}

void AnimationBuffer::AddData(const FrameInfo& FrameData)
{
    if(bBufferIsActive)
    {
        BufferData.push_back(FrameData);
    }
}

void AnimationBuffer::CleanOutdatedData()
{
    //Empty outdated data from the front of the buffer
    while(IsBufferFrontOutdated())
    {
        BufferData.pop_front();
    }
}

bool AnimationBuffer::IsBufferFrontOutdated()
{
    using namespace std::chrono;

    if(!BufferData.empty())
    {
        float FrontDuration = duration_cast<duration<float>>(steady_clock::now() - BufferData.front().GetTimeInfo().TimeCaptured).count();
        if(FrontDuration >= MaxBufferLength)
        {
            return true;
        }
    }

    return false;
}

std::vector<CarSeen> AnimationBuffer::GetCarsSeenInBuffer()
{
    std::vector<CarSeen> CarsSeenInBuffer;

    //Loop through each frame in the buffer
    for(const auto& DataPoint : BufferData)
    {
        //Loop through each of the cars this frame
        auto CarsSeenThisFrame = DataPoint.GetCarsSeen();
        for(const auto& CarSeenThisFrame : CarsSeenThisFrame)
        {
            //Compare this car to the cars in the buffer list
            bool bHasSeenCar = false;
            for(const auto& CarSeenInBuffer : CarsSeenInBuffer)
            {
                if(CarSeenThisFrame == CarSeenInBuffer)
                {
                    bHasSeenCar = true;
                    break;
                }
            }

            //If this car isn't in the list, add it
            if(!bHasSeenCar)
            {
                CarsSeenInBuffer.push_back(CarSeenThisFrame);
            }
        }
    }

    return CarsSeenInBuffer;
}
