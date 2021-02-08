#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"

namespace CBUtils
{
    std::string PrintFloat(const float InFloat, const int InDecimals);
    std::string PrintVector(const Vector& InVector, const int InDecimals);
    std::string PrintQuat(const Quat& InQuat, const int InDecimals);

    std::string GetCurrentTimeAsString();
}
