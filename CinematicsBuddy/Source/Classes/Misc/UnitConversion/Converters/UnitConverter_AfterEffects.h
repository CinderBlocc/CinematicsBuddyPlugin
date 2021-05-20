#pragma once
#include "IUnitConverter.h"

class UnitConverter_AfterEffects : public IUnitConverter
{
public:
    std::string GetProgramName() const override
    {
        return "AFTER EFFECTS";
    }

    Vector ConvertLocation(Vector Location) const override
    {
        Vector Output;

        Output.X = Location.Y *  2.54f;
        Output.Y = Location.Z * -2.54f;
        Output.Z = Location.X *  2.54f;

        return Output;
    }
};