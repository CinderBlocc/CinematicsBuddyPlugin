#include "AnimationExporter.h"
#include "SupportFiles/CBUtils.h"


/*

    @TODO:
        - Open and write to temp file while recording
        - Once recording is done, copy temp file into final file so that you now have a header you can write to
            - In order to do this, you need to keep track of data throughout the recording
                - i.e. number of frames, name of replay (get in StartRecording), all the player cars seen during the recording, etc

*/


// PATH VALIDATION //
std::filesystem::path AnimationExporter::GetExportPathFromString(const std::string& InPathName)
{
    std::filesystem::path OutputFilePath;

    if(InPathName.empty())
    {
        GlobalCvarManager->log("No special file path specified. Outputting to /bakkesmod/data/CinematicsBuddy/AnimationExports/");
        OutputFilePath = GlobalGameWrapper->GetBakkesModPath() / "data" / "CinematicsBuddy" / "AnimationExports";
    }
    else
    {
        //Ensure file path ends with a slash
        std::string FinalPathName = InPathName;
        if(InPathName.back() != '/' && InPathName.back() != '\'')
        {
            FinalPathName.append(1, '/');
        }

        GlobalCvarManager->log("Special file path specified. Outputting to " + FinalPathName);
        OutputFilePath = FinalPathName;
    }

    return OutputFilePath;
}


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
    std::filesystem::path OutputFilePath = GetExportPathFromString(InPathName);

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
    ActiveFile << "END" << std::endl;
    ActiveFile.close();

    ActiveCameraName = "";
}


// BUFFER RECORDING //
void AnimationExporter::StartBuffer()
{
    if(bBufferIsActive)
    {
        return;
    }

    BufferData.clear();
    bBufferIsActive = true;
}

void AnimationExporter::PauseBuffer()
{
    bBufferIsActive = false;
}

void AnimationExporter::StopBuffer()
{
    PauseBuffer();
    BufferData.clear();
}

void AnimationExporter::CaptureBuffer(const std::string& InPathName)
{
    //Open the file
    std::filesystem::path OutputFilePath = GetExportPathFromString(InPathName);
    OutputFilePath += "Buffer_" + CBUtils::GetCurrentTimeAsString() + EXTENSION_NAME;
    std::ofstream BufferFile(OutputFilePath);

    //Write to the file
    if(BufferFile.is_open())
    {
        //Create file header
        FileHeaderInfo FileHeader;

        /*
        
            FILL FILE HEADER INFO HERE
        
        */

        BufferFile << FileHeader.Print() << '\n';

        //Write the buffer data to the file
        for(const auto& DataPoint : BufferData)
        {
            BufferFile << DataPoint.Print() << '\n';
        }
        BufferFile << "END" << std::endl;
    }

    BufferFile.close();
}


// CAPTURE DATA ON TICK //
void AnimationExporter::Tick()
{
    if(!bRecording && !bBufferIsActive)
    {
        return;
    }

    FrameInfo FrameData = CaptureFrameData();

    if(bBufferIsActive)
    {
        BufferData.push_back(FrameData);
    }

    if(bRecording && ActiveFile.is_open())
    {
        ActiveFile << FrameData.Print() << '\n';
    }
}

FrameInfo AnimationExporter::CaptureFrameData()
{

}
