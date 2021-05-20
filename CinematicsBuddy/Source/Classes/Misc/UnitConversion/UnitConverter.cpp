#include "UnitConverter.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "Converters/IUnitConverter.h"
#include "Converters/UnitConverter_RL.h"
#include "Converters/UnitConverter_3dsMax.h"
#include "Converters/UnitConverter_AfterEffects.h"

UnitConverter::UnitConverter()
{
    Converters.emplace_back(new UnitConverter_RL());
    Converters.emplace_back(new UnitConverter_3dsMax());
    Converters.emplace_back(new UnitConverter_AfterEffects());

    MAKE_NOTIFIER(NOTIFIER_UNIT_CONVERT, ConvertUnits, "Prints current location of camera in each program's units");
}

UnitConverter::~UnitConverter()
{
    for(auto& Converter : Converters)
    {
        delete Converter;
        Converter = nullptr;
    }

    Converters.clear();
}

void UnitConverter::ConvertUnits()
{
    std::string LogOutput, ClipboardOutput;

    CameraWrapper Camera = GlobalGameWrapper->GetCamera();
    if(!Camera.IsNull())
    {
        Vector Location = Camera.GetLocation();
        LogOutput = "Converted units:\n";

        for(const auto& Converter : Converters)
        {
            ClipboardOutput += Converter->GetProgramName() + ": " + CBUtils::PrintVector(Converter->ConvertLocation(Location), 3, true) + "\n";
        }

        ClipboardOutput.pop_back();
    }
    else
    {
        ClipboardOutput = "Could not convert units. Camera does not exist.";
    }

    GlobalCvarManager->log(LogOutput + ClipboardOutput);

    //Copy output to the clipboard
    OpenClipboard(nullptr);
    EmptyClipboard();
    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, ClipboardOutput.size());
    if(!hg)
    {
        CloseClipboard();
        return;
    }
    memcpy(GlobalLock(hg), ClipboardOutput.c_str(), ClipboardOutput.size());
    GlobalUnlock(hg);
    SetClipboardData(CF_TEXT, hg);
    CloseClipboard();
    GlobalFree(hg);

    GlobalCvarManager->log("Copied to clipboard!");
}
