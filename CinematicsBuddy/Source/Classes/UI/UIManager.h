#pragma once
#include "UIElement.h"
#include <map>

class UIManager
{
public:
    static UIManager* GetInstance();
    static void DestroyInstance();

    void GenerateSettingsFile();
    void AddElement(const UIElement& NewElement) { m_Elements[NewElement.GetElementName()] = NewElement; }
    UIElement& EditElement(const std::string& ElementName) { return m_Elements[ElementName]; }

private:
    UIManager() = default;
    static UIManager* Instance_;
    std::map<std::string, UIElement> m_Elements;
};
