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
	time_t rawtime;
	struct tm *timestamp;
	char buffer [80];
	time(&rawtime);
	timestamp = localtime(&rawtime);
	strftime(buffer, 80, "%Y-%m-%d_%H-%M-%S", timestamp);

	return std::string(buffer);
}

std::filesystem::path CBUtils::GetExportPathFromString(const std::string& InPathName)
{
    std::filesystem::path OutputFilePath;

    if(InPathName.empty())
    {
        GlobalCvarManager->log("No special file path specified. Outputting to /bakkesmod/data/CinematicsBuddy/AnimationExports/");
        OutputFilePath = GlobalGameWrapper->GetBakkesModPath() / "data" / "CinematicsBuddy" / "AnimationExports";
    }
    else
    {
        //Ensure file path ends with a slash
        std::string FinalPathName = InPathName;
        if(InPathName.back() != '/' && InPathName.back() != '\'')
        {
            FinalPathName.append(1, '/');
        }

        GlobalCvarManager->log("Special file path specified. Outputting to " + FinalPathName);
        OutputFilePath = FinalPathName;
    }

    return OutputFilePath;
}
