#pragma once
#include <filesystem>

struct Vector;
struct Quat;

namespace CBUtils
{
    std::string PrintFloat(float InFloat, int MaxDecimals = 2);
    std::string PrintVector(const Vector& InVector, int MaxDecimals = 2);
    std::string PrintQuat(const Quat& InQuat, int MaxDecimals = 5);
    std::string PrintDoubleDigit(int InValue);

    std::string GetCurrentTimeAsString();
    std::filesystem::path GetExportPathFromString(const std::string& InPathName, bool bLogInfo = false);
    std::filesystem::path GetFinalFileName(std::filesystem::path IntendedPath, const std::string& InFileName, int IncrementLevel);
    std::string GetSpecialFilePath();
}
