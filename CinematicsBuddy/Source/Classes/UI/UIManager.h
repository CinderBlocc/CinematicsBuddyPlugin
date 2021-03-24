#pragma once
#include "UIElement.h"
#include <map>

class UIManager
{
public:
    void GenerateSettingsFile();
    void AddElement(const UIElement& NewElement) { Elements[NewElement.GetElementName()] = NewElement; }
    UIElement& EditElement(const std::string& ElementName) { return Elements[ElementName]; }

private:
    std::map<std::string, UIElement> Elements;
};
