#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"

namespace CBUtils
{
    std::string PrintFloat(float InFloat, int InDecimals = 2);
    std::string PrintVector(const Vector& InVector, int InDecimals = 2);
    std::string PrintQuat(const Quat& InQuat, int InDecimals = 6);

    std::string GetCurrentTimeAsString();
    std::filesystem::path GetExportPathFromString(const std::string& InPathName);
}
