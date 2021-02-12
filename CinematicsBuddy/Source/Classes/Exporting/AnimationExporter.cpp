#include "AnimationExporter.h"
#include "SupportFiles/CBUtils.h"
#include "DataCollectors/FrameInfo.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include <vector>

AnimationExporter::AnimationExporter()
{
    bIsRecording = false;
    MaxRecordingTime = 300.f;
    FramesInTempFile = 0;
}

void AnimationExporter::StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName)
{
    AnimationRecorder::StartRecording(InPathName, InFileName, InCameraName);

    //Don't restart the recording if it's already running
    if(bIsRecording)
    {
        return;
    }

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
    AnimationRecorder::StopRecording();
    
    if(!bIsRecording)
    {
        return;
    }

    TempFile.close();

    if(WriteFile(PendingPathName, PendingFileName, PendingCameraName))
    {
        //Remove temp file upon successful write of final file
        std::filesystem::remove(GetTempExportFilePath());
    }

    bIsRecording = false;
    GlobalCvarManager->getCvar(CVAR_IS_RECORDING_ACTIVE).setValue(false);
}

void AnimationExporter::AddData(const FrameInfo& FrameData)
{
    AnimationRecorder::AddData(FrameData);

    if(bIsRecording)
    {
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
