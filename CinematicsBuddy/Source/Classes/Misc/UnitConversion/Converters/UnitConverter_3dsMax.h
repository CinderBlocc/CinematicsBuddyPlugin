#pragma once
#include "IUnitConverter.h"

class UnitConverter_3dsMax : public IUnitConverter
{
public:
    std::string GetProgramName() const override
    {
        return "3DS MAX";
    }

    Vector ConvertLocation(Vector Location) const override
    {
        Vector Output = Location;

        Output.Y *= -1.f;

        return Output;
    }
};
