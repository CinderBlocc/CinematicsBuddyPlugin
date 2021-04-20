#include "AnimationRecorder.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "UI/UIManager.h"
#include "SimpleJSON/json.hpp"
#include "Misc/CBTimer.h"
#include <chrono>
#include <thread>

AnimationRecorder::AnimationRecorder(std::shared_ptr<UIManager> TheUI)
{
    UI = TheUI;

    //Register cvars only once
    if(!HaveCvarsBeenInitialzed())
    {
        //Register and bind cvars
        UI->AddElement({bIsFileWriting,      CVAR_IS_FILE_WRITING,  "##IsFileWriting",   "Handle UI state if file is writing", -1000001, -1000001, false, false});
        UI->AddElement({bIncrementFileNames, CVAR_INCREMENT_FILES,  "Automatically increment file names", "Automatically append a unique number to file names" });
        UI->AddElement({bSetSpecialPath,     CVAR_SET_SPECIAL_PATH, "##UseSpecialPath",  "Enable if you want to use a non-default path"                        });
        UI->AddElement({bSaveDollycamPath,   CVAR_SAVE_DOLLY_PATH,  "Save current dollycam path", "Saves current dollycam path next to the recording"          });
        UI->AddElement({SpecialPath,         CVAR_SPECIAL_PATH,     "##SpecialPath",     "Set the special export file path. Leave blank for default"           });
        UI->AddElement({FileName,            CVAR_FILE_NAME,        "File Name##Export", "Set the export file name"                                            });
        UI->AddElement({CameraName,          CVAR_CAMERA_NAME,      "Camera Name",       "Set the camera name"                                                 });
    }
    else
    {
        //Bind existing cvars
        GlobalCvarManager->getCvar(CVAR_IS_FILE_WRITING).bindTo(bIsFileWriting);
        GlobalCvarManager->getCvar(CVAR_INCREMENT_FILES).bindTo(bIncrementFileNames);
        GlobalCvarManager->getCvar(CVAR_SET_SPECIAL_PATH).bindTo(bSetSpecialPath);
        GlobalCvarManager->getCvar(CVAR_SAVE_DOLLY_PATH).bindTo(bSaveDollycamPath);
        GlobalCvarManager->getCvar(CVAR_SPECIAL_PATH).bindTo(SpecialPath);
        GlobalCvarManager->getCvar(CVAR_FILE_NAME).bindTo(FileName);
        GlobalCvarManager->getCvar(CVAR_CAMERA_NAME).bindTo(CameraName);
    }
}

bool AnimationRecorder::HaveCvarsBeenInitialzed()
{
    static bool bHaveBeenInitialized = false;

    if(!bHaveBeenInitialized)
    {
        bHaveBeenInitialized = true;
        return false;
    }

    return true;
}

void AnimationRecorder::OnMaxRecordingTimeChanged()
{
    //Stub function
}

void AnimationRecorder::StartRecording()
{
    //Stub function
}

void AnimationRecorder::StopRecording()
{
    //Stub function
}

void AnimationRecorder::AddData(const FrameInfo& FrameData)
{
    RecordedData.push_back(FrameData);
}

bool AnimationRecorder::WriteFile(StringParam InPathName, StringParam InFileName, StringParam InCameraName)
{
    //Check if the recording has any data
    if(RecordedData.empty())
    {
        GlobalCvarManager->log("ERROR: Cannot save file. No recorded data.");
        return false;
    }

    //Open the file
    std::filesystem::path OutputFilePath = CBUtils::GetExportPathFromString(InPathName, true);
    if(!std::filesystem::exists(OutputFilePath))
    {
        GlobalCvarManager->log("ERROR: Cannot save file. File path (" + OutputFilePath.string() + ") is invalid.");
        return false;
    }
    OutputFilePath = CBUtils::GetFinalFileName(OutputFilePath, InFileName, (*bIncrementFileNames ? 0 : -1));

    //Save current dollycam path next to the output file with the same name
    if(*bSaveDollycamPath)
    {
        SaveDollycamPath(OutputFilePath);
    }

    //Create a copy of the data to work from in the async task
    std::deque<FrameInfo> RecordedDataCopy = RecordedData;

    //Write to the file. Done in a separate thread so game doesn't freeze during writing
    std::thread WritingFile(&AnimationRecorder::WriteFileThread, this, OutputFilePath, InCameraName, RecordedDataCopy);
    WritingFile.detach();

    return true;
}

void AnimationRecorder::WriteFileThread(std::filesystem::path OutputFilePath, StringParam InCameraName, RecordingParam TheRecording)
{
    std::ofstream OutputFile(OutputFilePath);
    if(OutputFile.is_open())
    {
        //Lock the UI and certain functions while writing
        GlobalCvarManager->executeCommand(std::string(CVAR_IS_FILE_WRITING) + " 1", false);

        //Write the data to the file
        std::vector<CarSeen> CarsSeenInRecording = GetCarsSeenInRecording(TheRecording);
        WriteHeader(OutputFile, GetReplayMetadata(), InCameraName, TheRecording, CarsSeenInRecording);
        WriteRecordedDataToFile(OutputFile, TheRecording, CarsSeenInRecording);

        //Unlock the UI
        GlobalCvarManager->executeCommand(std::string(CVAR_IS_FILE_WRITING) + " 0", false);
        GlobalCvarManager->log("Successfully wrote file " + OutputFilePath.string());
    }
    else
    {
        GlobalCvarManager->log("ERROR: Could not open output file");
    }

    OutputFile.close();
}

void AnimationRecorder::WriteHeader(std::ofstream& FileStream, const ReplayMetadata& ReplayInfo, StringParam InCameraName, RecordingParam TheRecording, CarsSeenParam CarsSeenInRecording)
{
    //Write recording metadata
    FileStream << "RECORDING METADATA" << '\n';
    FileStream << "Version: "     << PLUGIN_VERSION << '\n';
    FileStream << "Camera: "      << InCameraName << '\n';
    FileStream << "Average FPS: " << CBUtils::PrintFloat(GetAverageFPS(TheRecording)) << '\n';
    FileStream << "Frames: "      << TheRecording.size() << '\n';
    FileStream << "Duration: "    << CBUtils::PrintFloat(GetRecordingDuration(TheRecording)) << '\n';
    FileStream << std::endl;

    //Write replay metadata if it exists and if currently in replay
    if(GetbWasWholeRecordingInSameReplay(TheRecording))
    {
        FileStream << "REPLAY METADATA" << '\n';
        FileStream << "Name: "   << ReplayInfo.ReplayName   << '\n';
        FileStream << "ID: "     << ReplayInfo.ReplayID     << '\n';
        FileStream << "Date: "   << ReplayInfo.ReplayDate   << '\n';
        FileStream << "FPS: "    << ReplayInfo.ReplayFPS    << '\n';
        FileStream << "Frames: " << ReplayInfo.ReplayFrames << '\n';
        FileStream << std::endl;
    }

    //Write cars seen
    if(!CarsSeenInRecording.empty())
    {
        int CarIndex = 0;
        json::JSON CarsSeenJSON = json::Object();
        for(const auto& Car : CarsSeenInRecording)
        {
            CarsSeenJSON[std::to_string(CarIndex)] = Car.ConvertToJSON();
            ++CarIndex;
        }

        FileStream << "CARS SEEN" << '\n';
        FileStream << CarsSeenJSON.dump(1, "\t") << '\n';
        FileStream << std::endl;
    }

    //Write example format
    FileStream << "EXAMPLE FRAME FORMAT" << '\n';
    FileStream << FrameInfo::PrintExampleFormat() << '\n';
    FileStream << std::endl;

    //Mark the header as complete so parsers know to start looping frames
    FileStream << "BEGIN ANIMATION" << std::endl;
}

void AnimationRecorder::WriteRecordedDataToFile(std::ofstream& FileStream, RecordingParam TheRecording, CarsSeenParam CarsSeenInRecording)
{
    const FrameInfo& FirstFrame = TheRecording[0];
    int FrameIndex = 0;
    for(const auto& DataPoint : TheRecording)
    {
        FileStream << DataPoint.Print(FirstFrame.GetTimeInfo(), FrameIndex, CarsSeenInRecording) << '\n';
        ++FrameIndex;
    }
}

float AnimationRecorder::GetRecordingDuration(RecordingParam TheRecording)
{
    //Need 2 data points to get a duration
    if(TheRecording.size() < 2)
    {
        return 0.f;
    }
    
    const TimeInfo& FirstFrame = TheRecording[0].GetTimeInfo();
    const TimeInfo& LastFrame = TheRecording.back().GetTimeInfo();

    return LastFrame.GetTimeDifference(FirstFrame);
}

float AnimationRecorder::GetAverageFPS(RecordingParam TheRecording)
{
    using namespace std::chrono;

    //Need at least 2 data points to get a time difference
    if(TheRecording.size() < 2)
    {
        return 0.f;
    }

    //Add up all the time differences between each frame
    float TimeDifferencesTotal = 0.f;
    for(size_t i = 1; i < TheRecording.size(); ++i)
    {
        auto TimeDifference = TheRecording[i].GetTimeInfo().TimeCaptured - TheRecording[i-1].GetTimeInfo().TimeCaptured;
        TimeDifferencesTotal += duration_cast<duration<float>>(TimeDifference).count();
    }

    //Get the average
    float AverageDifference = TimeDifferencesTotal / static_cast<float>(TheRecording.size());
    return 1.f / AverageDifference;
}

bool AnimationRecorder::GetbWasWholeRecordingInSameReplay(RecordingParam TheRecording)
{
    //No data. Definitely not in a replay
    if(TheRecording.empty())
    {
        return false;
    }

    //Get the replay ID of the first frame. If it is empty, it was not in a replay
    std::string ReplayID = TheRecording[0].GetReplayID();
    if(ReplayID.empty())
    {
        return false;
    }

    //Check if every frame's ID matches the first frame
    for(const auto& ThisFrame : TheRecording)
    {
        if(ThisFrame.GetReplayID() != ReplayID)
        {
            return false;
        }
    }

    return true;
}

ReplayMetadata AnimationRecorder::GetReplayMetadata()
{
    ReplayMetadata Output;

    if(!GlobalGameWrapper->IsInReplay()) { return Output; }
    ReplayServerWrapper Server = GlobalGameWrapper->GetGameEventAsReplay();
    if(Server.IsNull()) { return Output; }
    ReplayWrapper Replay = Server.GetReplay();
    if(Replay.memory_address == NULL) { return Output; }

    Output.ReplayName   = Replay.GetReplayName().IsNull() ? "" : Replay.GetReplayName().ToString();
    Output.ReplayID     = Replay.GetId().IsNull() ? "" : Replay.GetId().ToString();
    Output.ReplayDate   = Replay.GetDate().IsNull() ? "" : Replay.GetDate().ToString();
    Output.ReplayFPS    = CBUtils::PrintFloat(Replay.GetRecordFPS());
    Output.ReplayFrames = std::to_string(Replay.GetNumFrames());

    return Output;
}

std::vector<CarSeen> AnimationRecorder::GetCarsSeenInRecording(RecordingParam TheRecording)
{
    std::vector<CarSeen> CarsSeenInRecording;

    //Loop through each frame in the recorded data
    for(const auto& DataPoint : TheRecording)
    {
        //Loop through each of the cars this frame
        auto CarsSeenThisFrame = DataPoint.GetCarsSeen();
        for(const auto& CarSeenThisFrame : CarsSeenThisFrame)
        {
            if(CarSeenThisFrame.bIsNull)
            {
                continue;
            }

            //Compare this car to the cars in the already-seen list
            bool bHasSeenCar = false;
            for(const auto& CarSeenInRecording : CarsSeenInRecording)
            {
                if(CarSeenThisFrame == CarSeenInRecording)
                {
                    bHasSeenCar = true;
                    break;
                }
            }

            //If this car isn't in the list, add it
            if(!bHasSeenCar)
            {
                CarsSeenInRecording.push_back(CarSeenThisFrame);
            }
        }
    }

    return CarsSeenInRecording;
}

void AnimationRecorder::SaveDollycamPath(std::filesystem::path OutputFilePath)
{
    //Split OutputFilePath into its path and its file name
    std::string FolderPath;
    std::string FileName;
    FillDollycamOutputDirectory(OutputFilePath, FolderPath, FileName);

    //Store current value in dolly_path_directory, then assign the new path
    std::string PreviousDirectoryValue;
    CVarWrapper DollyDirectoryCvar = GlobalCvarManager->getCvar("dolly_path_directory");
    bool bDoesCvarExist = DollyDirectoryCvar.IsNull();
    if(bDoesCvarExist)
    {
        PreviousDirectoryValue = DollyDirectoryCvar.getStringValue();
        DollyDirectoryCvar.setValue(FolderPath);
    }
    
    //Call dolly_path_save with the file name
    GlobalCvarManager->executeCommand("dolly_path_save " + FileName);
    
    //Restore previous dolly_path_directory value
    if(bDoesCvarExist)
    {
        DollyDirectoryCvar.setValue(PreviousDirectoryValue);
    }
}

void AnimationRecorder::FillDollycamOutputDirectory(std::filesystem::path InPath, std::string& OutFolderPath, std::string& OutFileName)
{
    //Get the file name including the extension
    OutFileName = InPath.filename().u8string();

    //Remove the file name and extension from the end of the path
    std::string FullPathString = InPath.u8string();
    OutFolderPath = FullPathString.substr(0, FullPathString.size() - OutFileName.size());

    //Remove the extension from the file name
    OutFileName = OutFileName.substr(0, OutFileName.size() - std::string(EXTENSION_RECORDING).size());
}
