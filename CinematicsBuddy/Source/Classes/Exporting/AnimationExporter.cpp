#include "AnimationExporter.h"
#include "SupportFiles/CBUtils.h"
#include "DataCollectors/FrameInfo.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include <filesystem>
#include <vector>


/*

    @TODO:

        - Fix StartRecording so that it doesnt create the final file yet

        - Validate if the intended file can be created in StartRecording
            - Don't actually create it in that function, just check if it *can* be created
            - i.e. Check if the intended directory exists
            - If valid, store the pathname, filename, and cameraname to be used in StopRecording
        - Open and write to temp file while recording
        - Once recording is done, copy temp file into final file so that you now have a header you can write to
            - In order to do this, you need to keep track of data throughout the recording
                - i.e. number of frames, name of replay (get in StartRecording), all the player cars seen during the recording, etc

*/

using namespace std::chrono;

AnimationExporter::AnimationExporter()
{
    bIsRecording = false;
    MaxRecordingTime = 300.f;
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
    std::filesystem::path OutputFilePath = CBUtils::GetExportPathFromString(InPathName);

    //Get the file name. Open the file if a name is provided
    if(InFileName.empty())
    {
        GlobalCvarManager->log("ERROR: Cannot start recording. Please specify a file name");
        bCanStartRecording = false;
    }
    else
    {
        OutputFilePath += InFileName + EXTENSION_NAME;
        TempFile = std::ofstream(OutputFilePath);
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
        bIsRecording = true;
        TimeStartedRecording = steady_clock::now();

        PendingPathName   = InPathName;
        PendingFileName   = InFileName;
        PendingCameraName = InCameraName;

        /*
        
            @TODO: SET THE bRecording CVAR TO TRUE TO GREY OUT UI
        
        */
    }
}

void AnimationExporter::StopRecording()
{
    AnimationRecorder::StopRecording();

    /*
    
        @TODO
        - Close temp file
        - Get all the seen cars
        - Write the entirety of RecordedData to the final file
            - If that write was successful, delete temp file
        
        Set the bRecording cvar to false to ungrey UI

    */
}

void AnimationExporter::AddData(const FrameInfo& FrameData)
{
    AnimationRecorder::AddData(FrameData);

    if(bIsRecording)
    {
        //Write to the temp file in case a crash happens midway through recording
        //The temp file would be useful for debugging in those situations
        if(TempFile.is_open())
        {
            TempFile << FrameData.Print() << '\n';
        }

        float RecordingDuration = duration_cast<duration<float>>(steady_clock::now() - TimeStartedRecording).count();
        if(RecordingDuration > MaxRecordingTime)
        {
            StopRecording();
        }
    }
}
