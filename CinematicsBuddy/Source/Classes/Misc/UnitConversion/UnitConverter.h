#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"

class IUnitConverter;

class UnitConverter
{
public:
    UnitConverter();
    ~UnitConverter();

    void ConvertUnits();

private:
    std::vector<IUnitConverter*> Converters;
};
