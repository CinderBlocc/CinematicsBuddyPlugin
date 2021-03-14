#pragma once
#include <string>
#include <memory>

class AnimationImporter
{
public:
    AnimationImporter();

    void ApplyCameraAnimation();

private:
    std::shared_ptr<std::string> ImportFileName = std::make_shared<std::string>("");
    
    void ImportCameraAnimation();
    void ClearCameraAnimation();
};
