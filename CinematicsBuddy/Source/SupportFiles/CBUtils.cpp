#include "CBUtils.h"
#include "MacrosStructsEnums.h"
#include <sstream>
#include <iomanip>

std::string CBUtils::PrintFloat(float InFloat, int InDecimals)
{
    std::ostringstream Output;
	Output << std::fixed << std::setprecision(InDecimals) << InFloat;
	return Output.str();
}

std::string CBUtils::PrintVector(const Vector& InVector, int InDecimals)
{
    std::ostringstream Output;
	Output << std::fixed << std::setprecision(InDecimals) << InVector.X << "," << InVector.Y << "," << InVector.Z;
	return Output.str();
}

std::string CBUtils::PrintQuat(const Quat& InQuat, int InDecimals)
{
    std::ostringstream Output;
	Output << std::fixed << std::setprecision(InDecimals) << InQuat.W << "," << InQuat.X << "," << InQuat.Y << "," << InQuat.Z;
	return Output.str();
}

std::string CBUtils::GetCurrentTimeAsString()
{
	time_t RawTime;
	struct tm *Timestamp;
	char StringBuffer[80];
	time(&RawTime);
	Timestamp = localtime(&RawTime);
	strftime(StringBuffer, 80, "%Y-%m-%d_%H-%M-%S", Timestamp);

	return std::string(StringBuffer);
}

std::filesystem::path CBUtils::GetExportPathFromString(const std::string& InPathName, bool bLogInfo)
{
    std::filesystem::path OutputFilePath;

    if(InPathName.empty())
    {
        if(bLogInfo)
        {
            GlobalCvarManager->log("No special file path specified. Outputting to default /bakkesmod/data/CinematicsBuddy/AnimationExports/");
        }
        OutputFilePath = GlobalGameWrapper->GetDataFolder() / "CinematicsBuddy" / "AnimationExports";
        OutputFilePath += "/";
    }
    else
    {
        //Ensure file path ends with a slash
        std::string FinalPathName = InPathName;
        if(InPathName.back() != '/' && InPathName.back() != '\\')
        {
            FinalPathName.append(1, '/');
        }

        if(bLogInfo)
        {
            GlobalCvarManager->log("Special file path specified. Outputting to " + FinalPathName);
        }
        OutputFilePath = FinalPathName;
    }

    return OutputFilePath;
}
