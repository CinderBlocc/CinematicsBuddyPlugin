#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"

class IUnitConverter
{
public:
    virtual std::string GetProgramName() const            = 0;
    virtual Vector ConvertLocation(Vector Location) const = 0;
};
