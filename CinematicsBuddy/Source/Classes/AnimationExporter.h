#pragma once
#include "SupportFiles/MacrosStructsEnums.h"
#include <filesystem>
#include <fstream>
#include <vector>

class AnimationExporter
{
public:
    void StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName);
    void StopRecording();

    void StartBuffer();
    void PauseBuffer();
    void StopBuffer();
    void CaptureBuffer(const std::string& InPathName);
    
    void Tick();

private:
    bool bRecording;
    bool bBufferIsActive;

    std::ofstream ActiveFile;
    std::string ActiveCameraName;

    std::vector<FrameInfo> BufferData;

    std::filesystem::path GetExportPathFromString(const std::string& InPathName);
    FrameInfo CaptureFrameData();
};
