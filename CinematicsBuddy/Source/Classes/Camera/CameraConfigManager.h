#pragma once
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

class CameraConfigManager
{
public:
    CameraConfigManager();

private:
    std::shared_ptr<std::string> m_CurrentConfig = std::make_shared<std::string>("");
    std::shared_ptr<std::string> m_NewName       = std::make_shared<std::string>("");

    bool bApplyingConfig = false;

    std::vector<std::string> GetCvarList();
    void ResetManager();
    void ApplyConfig();
    void SaveConfig();
    void UpdateConfigList(bool bRefresh = true);
    std::filesystem::path GetConfigsFolder();
    std::string GetRelativeFilename(const std::filesystem::path& InPath);
};
