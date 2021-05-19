#pragma once
#include "IUnitConverter.h"

class UnitConverter_AfterEffects : public IUnitConverter
{
public:
    std::string GetProgramName() const override { return "AFTER EFFECTS"; }
    Vector  ConvertLocation(Vector  Location) const override;
    Rotator ConvertRotation(Rotator Rotation) const override;
};