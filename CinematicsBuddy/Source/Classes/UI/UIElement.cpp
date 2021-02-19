#include "UIElement.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "SupportFiles/CBUtils.h"

void UIElement::FillValues(StrParam InName, StrParam InUILabel, StrParam InDescription, float InMinVal, float InMaxVal, bool InbSearchable, bool InbSaveToCfg)
{
    ElementName = InName;
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
}

CVarWrapper UIElement::RegisterCvar()
{
    return GlobalCvarManager->registerCvar(ElementName, DefaultValue, Description, bSearchable, bHasMin, MinVal, bHasMax, MaxVal, bSaveToCfg);
}

void UIElement::AddDropdownOptions(const DropdownOptionsType& InOptions)
{
    DropdownOptions = InOptions;
}

std::string UIElement::Print(EUI ElementType, bool bInvertIfGrayedComponent) const
{
    switch(ElementType)
    {
        case EUI::Checkbox:    { return "1|"  + UILabel + "|" + ElementName; }
        case EUI::FloatRange:  { return "2|"  + UILabel + "|" + ElementName + "|" + CBUtils::PrintFloat(MinVal, 3) + "|" + CBUtils::PrintFloat(MaxVal, 3); }
        case EUI::IntRange:    { return "3|"  + UILabel + "|" + ElementName + "|" + std::to_string(static_cast<int>(MinVal)) + "|" + std::to_string(static_cast<int>(MaxVal)); }
        case EUI::Float:       { return "4|"  + UILabel + "|" + ElementName + "|" + CBUtils::PrintFloat(MinVal, 3) + "|" + CBUtils::PrintFloat(MaxVal, 3); }
        case EUI::Int:         { return "5|"  + UILabel + "|" + ElementName + "|" + std::to_string(static_cast<int>(MinVal)) + "|" + std::to_string(static_cast<int>(MaxVal)); }
        case EUI::Dropdown:    { return "6|"  + UILabel + "|" + ElementName + "|" + PrintOptions(); }
        case EUI::GrayedBegin: { std::string Header = "10|"; if(bInvertIfGrayedComponent) { Header += "!"; } return Header + ElementName; }
        case EUI::GrayedEnd:   { return "11|"; }
        case EUI::Textbox:     { return "12|" + UILabel + "|" + ElementName; }
        case EUI::ColorEdit:   { return "13|" + UILabel + "|" + ElementName; }
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
