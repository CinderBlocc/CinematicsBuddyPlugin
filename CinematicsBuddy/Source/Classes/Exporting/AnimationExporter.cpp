#include "AnimationExporter.h"
#include "DataCollectors/FrameInfo.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "UI/UIManager.h"
#include <vector>

AnimationExporter::AnimationExporter(std::shared_ptr<UIManager> TheUI)
    : AnimationRecorder(TheUI)
{
    //Register cvars
    UI->AddElement({RecordSize,         CVAR_MAX_RECORD_LENGTH,   "Max recording length (seconds)", "Number of seconds to record",        0,        1000                 });
    UI->AddElement({bIsRecordingActive, CVAR_IS_RECORDING_ACTIVE, "##RecordingActive", "Internal info about the state of the recording", -1000001, -1000001, false, false});
    
    //Bind an addOnValueChanged function to the recording time cvar
    ON_CVAR_CHANGED(CVAR_MAX_RECORD_LENGTH, AnimationExporter::OnMaxRecordingTimeChanged);
    OnMaxRecordingTimeChanged();

    //Register notifiers
    MAKE_NOTIFIER(NOTIFIER_RECORD_START, StartRecording, "Starts capturing animation data");
	MAKE_NOTIFIER(NOTIFIER_RECORD_STOP,  StopRecording,  "Stops capturing animation data");

    //Register hooks
    GlobalGameWrapper->HookEvent("Function ProjectX.EngineShare_X.EventPreLoadMap", std::bind(&AnimationExporter::OnNewMapLoading, this));
}

void AnimationExporter::OnMaxRecordingTimeChanged()
{
    MaxRecordingTime = *RecordSize;
}

void AnimationExporter::StartRecording()
{
    if(*bIsFileWriting)
    {
        return;
    }

    AnimationRecorder::StartRecording();

    //Don't restart the recording if it's already running
    if(bIsRecording)
    {
        return;
    }

    //Get useful parameters from cvars
    std::string InPathName   = CBUtils::GetSpecialFilePath();
    std::string InFileName   = *FileName;
    std::string InCameraName = *CameraName;

    //Let all the warnings get collected before returning
    bool bCanStartRecording = true;

    //Get the path
    std::filesystem::path AnticipatedFinalPath = CBUtils::GetExportPathFromString(InPathName);
    if(!std::filesystem::exists(AnticipatedFinalPath))
    {
        GlobalCvarManager->log("ERROR: Cannot start recording. File path (" + AnticipatedFinalPath.string() + ") is invalid");
        bCanStartRecording = false;
    }

    //Get the file name. Open the file if a name is provided
    if(InFileName.empty())
    {
        GlobalCvarManager->log("ERROR: Cannot start recording. Please specify a file name");
        bCanStartRecording = false;
    }

    //Get the camera name
    if(InCameraName.empty())
    {
        GlobalCvarManager->log("ERROR: Cannot start recording. Please specify a camera name");
        bCanStartRecording = false;
    }

    //If valid, start recording
    if(bCanStartRecording)
    {
        RecordedData.clear();
        bIsRecording = true;

        PendingPathName   = InPathName;
        PendingFileName   = InFileName;
        PendingCameraName = InCameraName;

        FramesInTempFile = 0;
        TempFile = std::ofstream(GetTempExportFilePath());

        GlobalCvarManager->getCvar(CVAR_IS_RECORDING_ACTIVE).setValue(true);
    }
}

void AnimationExporter::StopRecording()
{
    if(*bIsFileWriting)
    {
        return;
    }

    AnimationRecorder::StopRecording();
    
    if(!bIsRecording)
    {
        return;
    }

    bIsRecording = false;
    TempFile.close();

    if(WriteFile(PendingPathName, PendingFileName, PendingCameraName))
    {
        //Remove temp file upon successful write of final file
        std::filesystem::remove(GetTempExportFilePath());
    }

    GlobalCvarManager->getCvar(CVAR_IS_RECORDING_ACTIVE).setValue(false);
}

void AnimationExporter::OnNewMapLoading()
{
    //Game is about to switch to a new mode and destroy the current mode
    //In order to get replay metadata, the recording needs to stop here
    StopRecording();
}

void AnimationExporter::AddData(const FrameInfo& FrameData)
{
    if(bIsRecording)
    {
        AnimationRecorder::AddData(FrameData);

        const FrameInfo& FirstFrame = RecordedData.empty() ? FrameData : RecordedData[0];

        //Write to the temp file in case a crash happens midway through recording
        //The temp file will be useful for debugging in those situations
        if(TempFile.is_open())
        {
            TempFile << FrameData.Print(FirstFrame.GetTimeInfo(), FramesInTempFile, std::vector<CarSeen>()) << '\n';
            ++FramesInTempFile;
        }

        using namespace std::chrono;
        float RecordingDuration = duration_cast<duration<float>>(steady_clock::now() - FirstFrame.GetTimeInfo().TimeCaptured).count();
        if(RecordingDuration > MaxRecordingTime)
        {
            StopRecording();
        }
    }
}

std::filesystem::path AnimationExporter::GetTempExportFilePath()
{
    std::filesystem::path TempFilePath = CBUtils::GetExportPathFromString("");
    TempFilePath += "CBTempFile.tmp";

    return TempFilePath;
}
