#pragma once
#include <string>
#include <memory>
#include <vector>
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"

enum class EUI
{
    Button      = 0,
    Checkbox    = 1,
    FloatRange  = 2,
    IntRange    = 3,
    Float       = 4,
    Int         = 5,
    Dropdown    = 6,
    GrayedBegin = 10,
    GrayedEnd   = 11,
    Textbox     = 12,
    ColorEdit   = 13,
    UNKNOWN     = 100
};

class UIElement
{
public:
    using StrParam = const std::string&;
    using DropdownOptionsType = std::vector<std::pair<std::string, std::string>>;

    //Default constructor needed for std::map
    //Don't use this constructor, use the two below
    UIElement() = default;

    //Use this constructor when making a cvar
    template <typename T>
    UIElement(std::shared_ptr<T> InCvarPointer, StrParam InName, StrParam InUILabel, StrParam InDescription,
              float InMinVal = -1000001, float InMaxVal = -1000001, bool InbSearchable = true, bool InbSaveToCfg = true)
    { DoConstructCvar<T>(InCvarPointer, InName, InUILabel, InDescription, InMinVal, InMaxVal, InbSearchable, InbSaveToCfg); }

    //Use this constructor when making a notifier      // #TODO: Add this method. Macro the function lambda?
    //UIElement(StrParam InName, StrParam InUILabel, StrParam InDescription, function pointer, bool bReceivesParams);

    //Methods for generating UI
    void AddDropdownOptions(const DropdownOptionsType& InOptions);
    std::string Print(EUI PrintedType, bool bInvertIfGrayedComponent = false) const;
    std::string GetElementName() const { return ElementName; }

private:
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
    bool  bBindToCvar = true;
    DropdownOptionsType DropdownOptions;
    std::string PrintOptions() const;


    //Cvar constructor function definition
    template <typename T>
    void DoConstructCvar(std::shared_ptr<T> InCvarPointer, StrParam InName, StrParam InUILabel, StrParam InDescription,
                float InMinVal, float InMaxVal, bool InbSearchable, bool InbSaveToCfg)
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

        AssignDefaultValue<T>(InCvarPointer);
        RegisterCvar<T>(InCvarPointer);
    }

    template <typename T>
    void AssignDefaultValue(std::shared_ptr<T> InCvarPointer)
    {
        if(InCvarPointer)
        {
            DefaultValue = std::to_string(*InCvarPointer);
        }
    }

    template <>
    void AssignDefaultValue<std::string>(std::shared_ptr<std::string> InCvarPointer)
    {
        if(InCvarPointer)
        {
            DefaultValue = *InCvarPointer;
        }
    }

    template <typename T>
    void RegisterCvar(std::shared_ptr<T> InCvarPointer)
    {
        CVarWrapper NewCvar = GlobalCvarManager->registerCvar(ElementName, DefaultValue, Description, bSearchable, bHasMin, MinVal, bHasMax, MaxVal, bSaveToCfg);
        if(InCvarPointer)
        {
            NewCvar.bindTo(InCvarPointer);
        }
        GlobalCvarManager->log("Registered cvar (" + ElementName + ") with default value of (" + DefaultValue + ")");
    }
};
