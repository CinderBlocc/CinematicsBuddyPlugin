#include "UnitConverter.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "Converters/IUnitConverter.h"
#include "Converters/UnitConverter_RL.h"
#include "Converters/UnitConverter_3dsMax.h"
#include "Converters/UnitConverter_AfterEffects.h"

/*

    TODO:
        - Register notifier to convert units
        - Copy all output to clipboard and also print to console
            - In console print "Copied to clipboard!"
        - Create this class as a std::shared_ptr in CinematicsBuddy.cpp so it's constructor can be called
        - Add the PrintRotator function
            - Add enum to CBUtils near that function to choose whether to print as unreal units, degrees, or radians

*/

UnitConverter::UnitConverter()
{
    Converters.emplace_back(UnitConverter_RL());
    Converters.emplace_back(UnitConverter_3dsMax());
    Converters.emplace_back(UnitConverter_AfterEffects());
}

void UnitConverter::ConvertUnits(Vector InLocation, Rotator InRotation)
{
    std::string Output = "Converted units:\n";

    for(const auto& Converter : Converters)
    {
        Output += "\n" + Converter.GetProgramName() + "\n";
        Output += "\tLocation " + CBUtils::PrintVector(Converter.ConvertLocation(InLocation), 3) + "\n";
        Output += "\tRotation " + CBUtils::PrintRotator(Converter.ConvertRotation(InRotation), 3);
    }

    GlobalCvarManager->log(Output);

    /*
    
        COPY TO CLIPBOARD
    
    */

    GlobalCvarManager->log("Copied to clipboard!");
}
