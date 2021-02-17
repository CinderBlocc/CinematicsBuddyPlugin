#pragma once
#include <string>

class UIManager
{
public:
    void GenerateSettingsFile();

private:
    std::string GetBindingsList();
};
