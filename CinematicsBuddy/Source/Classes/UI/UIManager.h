#pragma once
#include "UIElement.h"
#include <map>

class UIManager
{
public:
    void GenerateSettingsFile();
    void AddElement(const UIElement& NewElement) { Elements[NewElement.ElementName] = NewElement; }

private:
    std::string GetBindingsList();

    std::map<std::string, UIElement> Elements;
};
