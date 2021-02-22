#pragma once
#include <memory>
#include <string>
#include <vector>

class UIManager;

class CameraConfigManager
{
public:
    CameraConfigManager(std::shared_ptr<UIManager> TheUI);

private:
    std::shared_ptr<UIManager> UI;

    std::shared_ptr<std::string> m_CurrentConfig = std::make_shared<std::string>("");
    std::shared_ptr<std::string> m_NewName       = std::make_shared<std::string>("");

    std::vector<std::string> GetCvarList();
    void ResetManager();
    void ApplyConfig();
    void SaveConfig();
    void UpdateConfigList();
};
