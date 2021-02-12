#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"

namespace CBUtils
{
    std::string PrintFloat(float InFloat, int MaxDecimals = 2);
    std::string PrintVector(const Vector& InVector, int MaxDecimals = 2);
    std::string PrintQuat(const Quat& InQuat, int MaxDecimals = 5);

    std::string GetCurrentTimeAsString();
    std::filesystem::path GetExportPathFromString(const std::string& InPathName, bool bLogInfo = false);
}
