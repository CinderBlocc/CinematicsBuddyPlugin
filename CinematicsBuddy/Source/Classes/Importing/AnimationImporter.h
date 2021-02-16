#pragma once
#include <string>
#include <memory>

class AnimationImporter
{
public:
    void InitCvars();
    void ImportCameraAnimation(const std::string& ImportFile);

private:
    std::shared_ptr<std::string> ImportFileName = std::make_shared<std::string>("");
};
