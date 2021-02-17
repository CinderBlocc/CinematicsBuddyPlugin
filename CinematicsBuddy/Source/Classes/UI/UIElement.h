#pragma once
#include <string>
#include <memory>

enum class UITypes
{
    
};

template <typename T>
class UIElement
{
public:
    using StrParam = const std::string&;
    UIElement(UITypes InType, StrParam InName, StrParam DefaultValue, 

    UITypes ElementType;
    std::string ElementName;

};
