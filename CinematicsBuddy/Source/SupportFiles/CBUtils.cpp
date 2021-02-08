#include "CBUtils.h"
#include <sstream>
#include <iomanip>

std::string CBUtils::PrintFloat(const float InFloat, const int InDecimals)
{
    std::ostringstream Output;
	Output << std::fixed << std::setprecision(InDecimals) << InFloat;
	return Output.str();
}

std::string CBUtils::PrintVector(const Vector& InVector, const int InDecimals)
{
    std::ostringstream Output;
	Output << std::fixed << std::setprecision(InDecimals) << InVector.X << ", " << InVector.Y << ", " << InVector.Z;
	return Output.str();
}

std::string CBUtils::PrintQuat(const Quat& InQuat, const int InDecimals)
{
    std::ostringstream Output;
	Output << std::fixed << std::setprecision(InDecimals) << InQuat.W << ", " << InQuat.X << ", " << InQuat.Y << ", " << InQuat.Z;
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
