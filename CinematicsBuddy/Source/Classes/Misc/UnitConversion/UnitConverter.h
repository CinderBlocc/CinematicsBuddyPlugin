#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"

class IUnitConverter;

class UnitConverter
{
public:
    UnitConverter();

    void ConvertUnits(Vector InLocation, Rotator InRotation);

private:
    std::vector<IUnitConverter> Converters;
};
