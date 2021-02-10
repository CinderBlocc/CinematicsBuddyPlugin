#include "AnimationBuffer.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "DataCollectors/FrameInfo.h"
#include "DataCollectors/FileHeaderInfo.h"
#include "SupportFiles/CBUtils.h"
#include <fstream>
#include <chrono>

/*

    @TODO:
    - Write buffer header before saving to file

    NOTES:
    - Don't end written file with "END". That will make JSON parsing harder to deal with

*/

AnimationBuffer::AnimationBuffer()
{
    bIsRecording = false;
    MaxRecordingTime = 30.f;
}

void AnimationBuffer::StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName)
{
    //Parameters aren't used (for now), but it's easier to have this in place in case they're needed later
    AnimationRecorder::StartRecording(InPathName, InFileName, InCameraName);

    if(bIsRecording)
    {
        return;
    }

    RecordedData.clear();
    bIsRecording = true;
}

void AnimationBuffer::StopRecording()
{
    AnimationRecorder::StopRecording();

    bIsRecording = false;
    RecordedData.clear();
}

void AnimationBuffer::CaptureBuffer(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName)
{
    if(RecordedData.empty())
    {
        return;
    }

    //Open the file
    std::filesystem::path OutputFilePath = CBUtils::GetExportPathFromString(InPathName);
    OutputFilePath += InFileName + "_Buffer_" + CBUtils::GetCurrentTimeAsString() + EXTENSION_NAME;
    std::ofstream BufferFile(OutputFilePath);

    //Write to the file
    if(BufferFile.is_open())
    {
        //Create file header
        FileHeaderInfo HeaderInfo;
        HeaderInfo.CarsSeenInRecording = GetCarsSeenInRecording();

        /*
        
            FILL FILE HEADER INFO HERE
        
        */

        //Write the header at the top of the file
        WriteHeader(BufferFile, HeaderInfo);

        //Write the buffer data to the file
        const FrameInfo& FirstFrame = RecordedData[0];
        for(const auto& DataPoint : RecordedData)
        {
            BufferFile << DataPoint.Print(FirstFrame.GetTimeInfo(), HeaderInfo.CarsSeenInRecording) << '\n';
        }
    }

    BufferFile.close();
}

void AnimationBuffer::AddData(const FrameInfo& FrameData)
{
    AnimationRecorder::AddData(FrameData);

    if(bIsRecording)
    {
        CleanOutdatedData();
    }
}

void AnimationBuffer::CleanOutdatedData()
{
    while(IsBufferFrontOutdated())
    {
        RecordedData.pop_front();
    }
}

bool AnimationBuffer::IsBufferFrontOutdated()
{
    using namespace std::chrono;

    if(!RecordedData.empty())
    {
        float FrontDuration = duration_cast<duration<float>>(steady_clock::now() - RecordedData[0].GetTimeInfo().TimeCaptured).count();
        if(FrontDuration >= MaxRecordingTime)
        {
            return true;
        }
    }

    return false;
}
