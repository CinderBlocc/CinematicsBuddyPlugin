#pragma once
#include <string>
#include <map>

class UIElement;

class UIManager
{
public:
    void GenerateSettingsFile();
    void AddElement(const UIElement& NewElement);

private:
    std::string GetBindingsList();
    std::string PrintElement(const std::string& ElementName);
    std::string MakeGreyedComponent(const std::string& BoolElementName, bool bInverted);

    std::map<std::string, UIElement> Elements;
};
