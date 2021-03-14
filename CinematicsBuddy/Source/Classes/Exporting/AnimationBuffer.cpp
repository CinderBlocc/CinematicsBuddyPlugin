#include "AnimationBuffer.h"
#include "DataCollectors/FrameInfo.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "UI/UIManager.h"
#include <chrono>

AnimationBuffer::AnimationBuffer()
{
    auto UI = UIManager::GetInstance();

    //Register cvars
    UI->AddElement({bIsBufferActive, CVAR_BUFFER_ENABLED,    "Enable Buffer",               "Enable constant recording buffer"    });
    UI->AddElement({BufferSize,      CVAR_MAX_BUFFER_LENGTH, "Max buffer length (seconds)", "Number of seconds to buffer", 0, 1000});
    
    //Bind addOnValueChanged functions to the enabled and recording time cvars
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
    if(!bIsRecording || *bIsFileWriting)
    {
        return;
    }

    //Get useful parameters from cvars
    std::string InPathName   = CBUtils::GetSpecialFilePath();
    std::string InFileName   = *FileName;
    std::string InCameraName = *CameraName;

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
