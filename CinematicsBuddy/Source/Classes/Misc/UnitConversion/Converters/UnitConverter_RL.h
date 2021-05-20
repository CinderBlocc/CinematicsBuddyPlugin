#pragma once
#include "IUnitConverter.h"

//This class exists for ease of iteration with the other converters
class UnitConverter_RL : public IUnitConverter
{
public:
    std::string GetProgramName() const override
    {
        return "ROCKET LEAGUE";
    }

    Vector ConvertLocation(Vector Location) const override
    {
        return Location;
    }
};