#pragma once
#include "IUnitConverter.h"

class UnitConverter_RL : public IUnitConverter
{
    std::string GetProgramName() const override { return "ROCKET LEAGUE"; }
    Vector  ConvertLocation(Vector  Location) const override { return Location; }
    Rotator ConvertRotation(Rotator Rotation) const override { return Rotation; }
};