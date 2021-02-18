#pragma once
#include "UIElement.h"
#include <map>

class UIManager
{
public:
    void GenerateSettingsFile();
    void AddElement(const UIElement& NewElement) { Elements[NewElement.GetElementName()] = NewElement; }

private:
    std::map<std::string, UIElement> Elements;
};
