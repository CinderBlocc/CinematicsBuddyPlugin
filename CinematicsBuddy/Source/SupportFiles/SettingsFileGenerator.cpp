#include "Main/CinematicsBuddy.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include <fstream>

#define nl(x) SettingsFile << std::string(x) << '\n'
#define blank SettingsFile << '\n'
#define cv(x) std::string(x)

void CinematicsBuddy::GenerateSettingsFile()
{
    std::filesystem::path SettingsFilePath = GlobalGameWrapper->GetBakkesModPath() / "plugins" / "settings";
    if(!std::filesystem::exists(SettingsFilePath))
    {
        GlobalCvarManager->log("ERROR: Failed to write settings file. Path (" + SettingsFilePath.string() + ") does not exist");
        return;
    }

    std::ofstream SettingsFile(SettingsFilePath / "CinematicsBuddy.set");
    if(!SettingsFile.is_open())
    {
        GlobalCvarManager->log("ERROR: Failed to write settings file. File could not be opened");
        return;
    }

    nl("Cinematics Buddy");
    blank;
    nl("9|Version: " + cv(PLUGIN_VERSION));
    
    blank;
    nl("8|");
    blank;
    
    nl("10|!" + cv(CVAR_IS_RECORDING_ACTIVE));
        nl("9|NORMAL RECORDING");
        nl("12|File Name##Export|" + cv(CVAR_FILE_NAME));
        nl("12|Camera Name|" + cv(CVAR_CAMERA_NAME));
        nl("1|##UseSpecialPath|" + cv(CVAR_SET_SPECIAL_PATH));
        nl("7|");
        nl("10|" + cv(CVAR_SET_SPECIAL_PATH));
            nl("9|Special Path");
            nl("7|");
            nl("12|##SpecialPath|" + cv(CVAR_SPECIAL_PATH));
            nl("9|NOTE: Unchecking \"Special Path\", or leaving the path textbox blank will use the default /data/CinematicsBuddy/AnimationExports/ folder.");
        nl("11|");
        nl("0|Start recording|" + cv(NOTIFIER_RECORD_START));
        nl("7|");
    nl("11|");
    nl("10|" + cv(CVAR_IS_RECORDING_ACTIVE));
        nl("0|Stop recording|" + cv(NOTIFIER_RECORD_STOP));
    nl("11|");
    nl("4|Max recording length (seconds)|" + cv(CVAR_MAX_RECORD_LENGTH) + "|0|600");

    blank;
    nl("8|");
    blank;

    nl("9|BUFFER RECORDING");
    nl("0|Start Buffer|" + cv(NOTIFIER_BUFFER_START));
    nl("7|");
    nl("0|Capture Buffer|" + cv(NOTIFIER_BUFFER_CAPTURE));
    nl("7|");
    nl("0|Cancel Buffer|" + cv(NOTIFIER_BUFFER_STOP));
    nl("4|Max buffer length (seconds)|" + cv(CVAR_MAX_BUFFER_LENGTH) + "|0|600");
    nl("9|NOTE: File is saved to location specified by \"Special Path\" in the NORMAL RECORDING section. Read the note there for instructions.");
    
    blank;
    nl("8|");
    blank;
    
    nl("9|IMPORTING");
    nl("7|");
    nl("12|File Name##Import|" + cv(CVAR_IMPORT_FILE_NAME));
    nl("0|Import Selected Animation|" + cv(NOTIFIER_IMPORT_FILE));
    nl("7|");
    nl("0|Clear Animation|" + cv(NOTIFIER_IMPORT_CLEAR));
    
    blank;
    nl("8|");
    blank;
    
    nl("9|CAMERA OVERRIDES");
    nl("1|Enable Overrides. NOTE: This may conflict with SpectatorControls. Recommend disabling those overrides while using these.|" + cv(CVAR_ENABLE_CAM_OVERRIDE));
    nl("10|" + cv(CVAR_ENABLE_CAM_OVERRIDE));
        nl("4|Camera Movement Speed|" + cv(CVAR_CAM_MOVEMENT_SPEED) + "|0|3");
        nl("4|Camera Rotation Speed|" + cv(CVAR_CAM_ROTATION_SPEED) + "|0|3");
    nl("11|");
    

    SettingsFile.close();
    GlobalCvarManager->executeCommand("cl_settings_refreshplugins");
}
