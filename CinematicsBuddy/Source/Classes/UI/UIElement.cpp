#include "UIElement.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "SupportFiles/CBUtils.h"

void UIElement::AddDropdownOptions(const DropdownOptionsType& InOptions)
{
    DropdownOptions = InOptions;
}

std::string UIElement::Print(EUI ElementType, bool bInvertIfGrayedComponent) const
{
    switch(ElementType)
    {
        case EUI::Button:      { return "0|"  + ElementName; }
        case EUI::Checkbox:    { return "1|"  + ElementName; }
        case EUI::FloatRange:  { return "2|"  + ElementName + "|" + CBUtils::PrintFloat(MinVal, 3) + "|" + CBUtils::PrintFloat(MaxVal, 3); }
        case EUI::IntRange:    { return "3|"  + ElementName + "|" + std::to_string(static_cast<int>(MinVal)) + "|" + std::to_string(static_cast<int>(MaxVal)); }
        case EUI::Float:       { return "4|"  + ElementName + "|" + CBUtils::PrintFloat(MinVal, 3) + "|" + CBUtils::PrintFloat(MaxVal, 3); }
        case EUI::Int:         { return "5|"  + ElementName + "|" + std::to_string(static_cast<int>(MinVal)) + "|" + std::to_string(static_cast<int>(MaxVal)); }
        case EUI::Dropdown:    { return "6|"  + ElementName + "|" + PrintOptions(); }
        case EUI::GrayedBegin: { std::string Header = "10|"; if(bInvertIfGrayedComponent) { Header += "!"; } return Header + ElementName; }
        case EUI::GrayedEnd:   { return "11|"; }
        case EUI::Textbox:     { return "12|" + ElementName; }
        case EUI::ColorEdit:   { return "13|" + ElementName; }
        default: { return ""; }
    }
}

std::string UIElement::PrintOptions() const
{
    std::string Output;

    //Compile list into one string
    for(const auto& DropdownOption : DropdownOptions)
    {
        Output += DropdownOption.first + "@" + DropdownOption.second + "&";
    }

    //Remove last "&"
    Output.pop_back();

    return Output;
}
