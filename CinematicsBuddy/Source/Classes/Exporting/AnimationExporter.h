#pragma once
#include <fstream>

class FrameInfo;

class AnimationExporter
{
public:
    void StartRecording(const std::string& InPathName, const std::string& InFileName, const std::string& InCameraName);
    void StopRecording();
    
    bool GetbIsRecording() { return bRecording; }
    void AddData(const FrameInfo& FrameData);

private:
    bool bRecording;

    std::ofstream ActiveFile;
    std::string ActiveCameraName;
};
