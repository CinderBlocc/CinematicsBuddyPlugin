#include "CBUtils.h"
#include "MacrosStructsEnums.h"
#include <sstream>
#include <iomanip>

std::string CBUtils::PrintFloat(float InFloat, int MaxDecimals)
{
    std::ostringstream Output;
    float MinThreshold = powf(.1f, static_cast<float>(MaxDecimals));

    if(MaxDecimals == 0 || (InFloat - static_cast<int>(InFloat)) < MinThreshold)
    {
        //All decimals were 0. Print as integer
        Output << static_cast<int>(InFloat);
    }
    else
    {
        //This probably slows down printing, but it saves file space
        //Start at 1 because 0 was already tested in the if statement above
        int CalculatedDecimals = MaxDecimals;
        for(int i = 1; i <= MaxDecimals; ++i)
        {
            //Shift all decimals up
            float Shifted = InFloat * powf(10.f, static_cast<float>(i));

            //Remove the integer component and see if any decimals remain
            float Trimmed = Shifted - static_cast<int>(Shifted);
            if(Trimmed < MinThreshold)
            {
                //No decimals remain after this point
                CalculatedDecimals = i;
                break;
            }
        }

	    Output << std::fixed << std::setprecision(CalculatedDecimals) << InFloat;
    }

	return Output.str();
}

std::string CBUtils::PrintVector(const Vector& InVector, int MaxDecimals)
{
    std::string Output;
    
    Output = PrintFloat(InVector.X, MaxDecimals) + ","
           + PrintFloat(InVector.Y, MaxDecimals) + ","
           + PrintFloat(InVector.Z, MaxDecimals);

	return Output;
}

std::string CBUtils::PrintQuat(const Quat& InQuat, int MaxDecimals)
{
    std::string Output;
    
    Output = PrintFloat(InQuat.W, MaxDecimals) + ","
           + PrintFloat(InQuat.X, MaxDecimals) + ","
           + PrintFloat(InQuat.Y, MaxDecimals) + ","
           + PrintFloat(InQuat.Z, MaxDecimals);

	return Output;
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
