#include "AnimationBuffer.h"
#include "DataCollectors/FrameInfo.h"
#include "SupportFiles/CBUtils.h"
#include <chrono>

AnimationBuffer::AnimationBuffer()
{
    bIsRecording = false;
    MaxRecordingTime = 30.f;
}

void AnimationBuffer::StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName)
{
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
    if(!bIsRecording)
    {
        return;
    }

    std::string BufferFileName = InFileName + "_Buffer_" + CBUtils::GetCurrentTimeAsString();
    WriteFile(InPathName, BufferFileName, InCameraName);
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
