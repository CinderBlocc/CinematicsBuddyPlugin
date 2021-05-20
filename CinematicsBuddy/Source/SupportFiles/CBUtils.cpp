#include "CBUtils.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "MacrosStructsEnums.h"
#include <sstream>
#include <iomanip>

std::string CBUtils::PrintFloat(float InFloat, int MaxDecimals)
{
    std::ostringstream Output;
    float MinThreshold = powf(.1f, static_cast<float>(MaxDecimals));

    if(MaxDecimals == 0 || abs(InFloat - static_cast<int>(InFloat)) < MinThreshold)
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
            if(abs(Trimmed) < MinThreshold)
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

std::string CBUtils::PrintVector(const Vector& InVector, int MaxDecimals, bool bPrintSpaces)
{
    std::string Output;
    
    std::string Separator = bPrintSpaces ? ", " : ",";

    Output = PrintFloat(InVector.X, MaxDecimals) + Separator
           + PrintFloat(InVector.Y, MaxDecimals) + Separator
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

std::string CBUtils::PrintDoubleDigit(int InValue)
{
    //Clamp to double digits
    InValue = min(InValue, 99);

    //Already double digits
    if(InValue >= 10)
    {
        return std::to_string(InValue);
    }

    return "0" + std::to_string(InValue);
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

std::filesystem::path CBUtils::GetFinalFileName(std::filesystem::path IntendedPath, const std::string& InFileName, int IncrementLevel)
{
    std::filesystem::path Output = IntendedPath;

    //No auto-increment, just write the file with the direct name
    if(IncrementLevel < 0)
    {
        Output += InFileName + EXTENSION_RECORDING;
        return Output;
    }

    std::filesystem::path CheckIfExists = Output;
    if(IncrementLevel == 0)
    {
        //If _00 just print the normal name
        CheckIfExists += InFileName;
    }
    else
    {
        //It could be confusing to have "FILENAME" and "FILENAME_01", so skip straight to "FILENAME_02"
        if(IncrementLevel == 1)
        {
            IncrementLevel = 2;
        }

        //Append _01 - _99. If increment is more than 99 somehow, it'll just overwrite 99
        CheckIfExists += InFileName + "_" + PrintDoubleDigit(IncrementLevel);
    }

    CheckIfExists += EXTENSION_RECORDING;
    if(std::filesystem::exists(CheckIfExists))
    {
        return GetFinalFileName(IntendedPath, InFileName, ++IncrementLevel);
    }
    
    Output = CheckIfExists;
    return Output;
}

std::string CBUtils::GetSpecialFilePath()
{
    if(GlobalCvarManager->getCvar(CVAR_SET_SPECIAL_PATH).getBoolValue())
    {
        return GlobalCvarManager->getCvar(CVAR_SPECIAL_PATH).getStringValue();
    }
    
    return "";
}
