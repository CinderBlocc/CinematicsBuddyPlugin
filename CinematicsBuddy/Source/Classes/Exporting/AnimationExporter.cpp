#include "AnimationExporter.h"
#include "SupportFiles/CBUtils.h"
#include "DataCollectors/FrameInfo.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include <filesystem>
#include <vector>


/*

    @TODO:
        - Open and write to temp file while recording
        - Once recording is done, copy temp file into final file so that you now have a header you can write to
            - In order to do this, you need to keep track of data throughout the recording
                - i.e. number of frames, name of replay (get in StartRecording), all the player cars seen during the recording, etc

*/


//  NORMAL RECORDING //
void AnimationExporter::StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName)
{
    //Don't restart the recording if it's already running
    if(bRecording)
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
        ActiveFile = std::ofstream(OutputFilePath);
    }

    //Get the camera name
    if(InCameraName.empty())
    {
        GlobalCvarManager->log("ERROR: Cannot start recording. Please specify a camera name");
        bCanStartRecording = false;
    }
    else
    {
        ActiveCameraName = InCameraName;
    }

    //If valid, start recording
    if(bCanStartRecording)
    {
        bRecording = true;
    }
}

void AnimationExporter::StopRecording()
{
    /*
    
        Need to close temp file, then swap all data to the specified file and delete temp file
    
    */

    ActiveFile << "END" << std::endl;
    ActiveFile.close();

    ActiveCameraName = "";
}

void AnimationExporter::AddData(const FrameInfo& FrameData)
{
    if(bRecording && ActiveFile.is_open())
    {
        ActiveFile << FrameData.Print() << '\n';
    }
}
