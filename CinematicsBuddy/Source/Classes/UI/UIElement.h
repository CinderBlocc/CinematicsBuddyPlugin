#pragma once
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"

enum class EUI
{
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
    UIElement() = default; //Default constructor needed for std::map. Don't use this constructor, use the one below.

    //CVAR CONSTRUCTOR
    template <typename T>
    UIElement(std::shared_ptr<T> InCvarPointer, StrParam InName, StrParam InUILabel, StrParam InDescription,
              float InMinVal = -1000001, float InMaxVal = -1000001, bool InbSearchable = true, bool InbSaveToCfg = true)
    { DoConstructCvar<T>(InCvarPointer, InName, InUILabel, InDescription, InMinVal, InMaxVal, InbSearchable, InbSaveToCfg); }

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


    //Cvar constructor function definitions
    CVarWrapper RegisterCvar();
    void FillValues(StrParam InName, StrParam InUILabel, StrParam InDescription, float InMinVal, float InMaxVal, bool InbSearchable, bool InbSaveToCfg);

    template <typename T>
    void DoConstructCvar(std::shared_ptr<T> InCvarPointer, StrParam InName, StrParam InUILabel, StrParam InDescription,
                float InMinVal, float InMaxVal, bool InbSearchable, bool InbSaveToCfg)
    {
        //Fill the class members
        FillValues(InName, InUILabel, InDescription, InMinVal, InMaxVal, InbSearchable, InbSaveToCfg);
        
        if(InCvarPointer)
        {
            //Get the default value
            std::ostringstream TheString;
            TheString << *InCvarPointer;
            DefaultValue = TheString.str();

            //Register and bind the cvar
            RegisterCvar().bindTo(InCvarPointer);
        }
        else
        {
            //Only register the cvar
            RegisterCvar();
        }

        GlobalCvarManager->log("Registered cvar (" + ElementName + ") with default value of (" + DefaultValue + ")");
    }
};
