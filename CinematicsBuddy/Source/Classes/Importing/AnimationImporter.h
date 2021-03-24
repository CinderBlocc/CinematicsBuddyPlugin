#pragma once
#include <string>
#include <memory>

class UIManager;

class AnimationImporter
{
public:
    AnimationImporter(std::shared_ptr<UIManager> TheUI);

    void ApplyCameraAnimation();

private:
    std::shared_ptr<std::string> ImportFileName = std::make_shared<std::string>("");
    std::shared_ptr<UIManager> UI;
    
    void ImportCameraAnimation();
    void ClearCameraAnimation();
};
