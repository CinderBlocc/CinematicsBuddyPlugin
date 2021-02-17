#pragma once
#include <string>
#include <memory>
#include <vector>

enum class UIType
{
    //Specifying numbers because not all UI types require a cvar
    //Ones that don't require cvars are skipped
    Checkbox   = 1,
    FloatRange = 2,
    IntRange   = 3,
    Float      = 4,
    Int        = 5,
    Dropdown   = 6,
    Textbox    = 12,
    ColorEdit  = 13,
    UNKNOWN    = 100
};

class UIElement
{
public:
    using StrParam = const std::string&;
    using DropdownOptionsType = std::vector<std::pair<std::string, std::string>>;
    
    UIElement() = default;

    template <typename T>
    UIElement(UIType InType, std::shared_ptr<T> InCvarPointer, StrParam InName, StrParam InUILabel, StrParam InDescription,
              bool bBindToCvar = true, float InMinVal = -1000001, float InMaxVal = -1000001, bool InbSearchable = true, bool InbSaveToCfg = true);

    void AddDropdownOptions(const DropdownOptionsType& InOptions);
    std::string PrintUI();
    std::string PrintOptions();
    std::string MakeGrayedComponent(bool bInverted = false);

    UIType ElementType = UIType::UNKNOWN;
    std::string ElementName;
    std::string DefaultValue;
    std::string UILabel;
    std::string Description;
    bool  bHasMin     = false;
    float MinVal      = 0.f;
    bool  bHasMax     = false;
    float MaxVal      = 0.f;
    bool  bSearchable = true;
    bool  bSaveToCfg  = true;
    DropdownOptionsType DropdownOptions;
};
