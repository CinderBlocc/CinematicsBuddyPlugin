#include "UIElement.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "SupportFiles/CBUtils.h"

template <typename T>
UIElement::UIElement(UIType InType, std::shared_ptr<T> InCvarPointer, StrParam InName, StrParam InUILabel, StrParam InDescription,
                     bool bBindToCvar /*= true*/, float InMinVal /*= -1000001*/, float InMaxVal /*= -1000001*/, bool InbSearchable /*= true*/, bool InbSaveToCfg /*= true*/)
{
    ElementType = InType;
    ElementName = InName;
    DefaultValue = std::string(*InCvarPointer);
    UILabel = InUILabel;
    Description = InDescription;
    if(InMinVal >= -1000000)
    {
        bHasMin = true;
        MinVal = InMinVal;
    }
    if(InMaxVal >= -1000000)
    {
        bHasMax = true;
        MaxVal = InMaxVal;
    }
    bSearchable = InbSearchable;
    bSaveToCfg = InbSaveToCfg;


    //Register cvar
    CVarWrapper NewCvar = GlobalCvarManager->registerCvar(ElementName, DefaultValue, Description, bSearchable, bHasMin, MinVal, bHasMax, MaxVal, bSaveToCfg);
    if(bBindToCvar)
    {
        NewCvar.bindTo(InCvarPointer);
    }
    GlobalCvarManager->log("Registered cvar (" + ElementName + ") with default value of (" + DefaultValue + ")");
}

void UIElement::AddDropdownOptions(const DropdownOptionsType& InOptions)
{
    DropdownOptions = InOptions;
}

std::string UIElement::PrintUI()
{
    switch(ElementType)
    {
        case UIType::Checkbox:   { return "1|"  + ElementName; }
        case UIType::FloatRange: { return "2|"  + ElementName + "|" + CBUtils::PrintFloat(MinVal, 3) + "|" + CBUtils::PrintFloat(MaxVal, 3); }
        case UIType::IntRange:   { return "3|"  + ElementName + "|" + std::to_string(static_cast<int>(MinVal)) + "|" + std::to_string(static_cast<int>(MaxVal)); }
        case UIType::Float:      { return "4|"  + ElementName + "|" + CBUtils::PrintFloat(MinVal, 3) + "|" + CBUtils::PrintFloat(MaxVal, 3); }
        case UIType::Int:        { return "5|"  + ElementName + "|" + std::to_string(static_cast<int>(MinVal)) + "|" + std::to_string(static_cast<int>(MaxVal)); }
        case UIType::Dropdown:   { return "6|"  + ElementName + "|" + PrintOptions(); }
        case UIType::Textbox:    { return "12|" + ElementName; }
        case UIType::ColorEdit:  { return "13|" + ElementName; }
        default: { return ""; }
    }
}

std::string UIElement::PrintOptions()
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

std::string UIElement::MakeGrayedComponent(bool bInverted)
{
    std::string Header = bInverted ? "10|!" : "10|";
    return Header + ElementName;
}

