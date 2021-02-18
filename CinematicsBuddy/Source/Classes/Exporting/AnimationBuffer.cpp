#include "AnimationBuffer.h"
#include "DataCollectors/FrameInfo.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "UI/UIManager.h"
#include <chrono>

AnimationBuffer::AnimationBuffer(std::shared_ptr<UIManager> TheUI)
    : AnimationRecorder(TheUI)
{
    //Register cvars
	MAKE_CVAR_BIND_TO_STRING(bIsBufferActive, CVAR_BUFFER_ENABLED, "Enable constant recording buffer", false);
    MAKE_CVAR_BIND_TO_STRING(BufferSize, CVAR_MAX_BUFFER_LENGTH, "Number of seconds to buffer", true, true, 0, true, 1000);
    ON_CVAR_CHANGED(CVAR_BUFFER_ENABLED, AnimationBuffer::OnBufferEnabledChanged);
    ON_CVAR_CHANGED(CVAR_MAX_BUFFER_LENGTH, AnimationBuffer::OnMaxRecordingTimeChanged);
    OnMaxRecordingTimeChanged();

    //Register notifiers
    MAKE_NOTIFIER(NOTIFIER_BUFFER_CAPTURE, CaptureBuffer, "Captures the data in the buffer");
	MAKE_NOTIFIER(NOTIFIER_BUFFER_CLEAR,   ClearBuffer,   "Cleares the data from the buffer");
}

void AnimationBuffer::OnBufferEnabledChanged()
{
    if(*bIsBufferActive)
    {
        StartRecording();
    }
    else
    {
        StopRecording();
    }
}

void AnimationBuffer::OnMaxRecordingTimeChanged()
{
    MaxRecordingTime = *BufferSize;
}

void AnimationBuffer::StartRecording()
{
    AnimationRecorder::StartRecording();

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

void AnimationBuffer::CaptureBuffer()
{
    if(!bIsRecording)
    {
        return;
    }

    //Get useful parameters from cvars
    std::string InPathName   = CBUtils::GetSpecialFilePath();
    std::string InFileName   = GlobalCvarManager->getCvar(CVAR_FILE_NAME).getStringValue();
    std::string InCameraName = GlobalCvarManager->getCvar(CVAR_CAMERA_NAME).getStringValue();

    std::string BufferFileName = InFileName + "_Buffer_" + CBUtils::GetCurrentTimeAsString();
    WriteFile(InPathName, BufferFileName, InCameraName);
}

void AnimationBuffer::ClearBuffer()
{
    RecordedData.clear();
}

void AnimationBuffer::AddData(const FrameInfo& FrameData)
{
    if(bIsRecording)
    {
        AnimationRecorder::AddData(FrameData);
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
