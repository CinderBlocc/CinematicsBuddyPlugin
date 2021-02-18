#include "UIManager.h"
#include "UIElement.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include <filesystem>
#include <fstream>

#define nl(x) SettingsFile << std::string(x) << '\n'
#define nl2(x, y) nl(Elements[y].Print(x))
#define nl3(x, y, z) nl(Elements[y].Print(x, z))
#define cv(x) std::string(x)
#define blank SettingsFile << '\n'
#define sameline nl("7|")
#define separator nl("8|")
#define label(x) nl(std::string("9|") + x)

void UIManager::GenerateSettingsFile()
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

    label("Version: " + cv(PLUGIN_VERSION));
    nl("10|" + cv(CVAR_IS_FILE_WRITING));
        nl("10|" + cv(CVAR_IS_FILE_WRITING));
            nl("10|" + cv(CVAR_IS_FILE_WRITING));
                nl("10|" + cv(CVAR_IS_FILE_WRITING));
                    nl("10|" + cv(CVAR_IS_FILE_WRITING));
                        label("File is writing! Please wait for it to finish before continuing.");
                    nl("11|");
                nl("11|");
            nl("11|");
        nl("11|");
    nl("11|");
    
    blank;
    separator;
    blank;

    nl("10|!" + cv(CVAR_IS_FILE_WRITING));

        nl("10|!" + cv(CVAR_IS_RECORDING_ACTIVE));
            label("NORMAL RECORDING");
            nl("1|Automatically increment file names|" + cv(CVAR_INCREMENT_FILES));
            nl("12|File Name##Export|" + cv(CVAR_FILE_NAME));
            nl("12|Camera Name|" + cv(CVAR_CAMERA_NAME));
            nl("1|##UseSpecialPath|" + cv(CVAR_SET_SPECIAL_PATH));
            sameline;
            nl("10|" + cv(CVAR_SET_SPECIAL_PATH));
                label("Special Path");
                sameline;
                nl("12|##SpecialPath|" + cv(CVAR_SPECIAL_PATH));
                label("NOTE: Unchecking \"Special Path\", or leaving the path textbox blank will use the default /data/CinematicsBuddy/AnimationExports/ folder.");
                label("Recommended setting: leave it disabled and let the plugin handle the paths automatically");
            nl("11|");//SetSpecialPath
            nl("0|Start recording|" + cv(NOTIFIER_RECORD_START));
            sameline;
        nl("11|");//IsRecordingActive
        nl("10|" + cv(CVAR_IS_RECORDING_ACTIVE));
            nl("10|!" + cv(CVAR_IS_FILE_WRITING));
                nl("0|Stop recording|" + cv(NOTIFIER_RECORD_STOP));
            nl("11|");//!IsFileWriting
        nl("11|");//IsRecordingActive
        nl("4|Max recording length (seconds)|" + cv(CVAR_MAX_RECORD_LENGTH) + "|0|600");

        blank;
        separator;
        blank;

        label("BUFFER RECORDING");
        nl("1|Enable Buffer|" + cv(CVAR_BUFFER_ENABLED));
        sameline;
        nl("0|Capture Buffer|" + cv(NOTIFIER_BUFFER_CAPTURE));
        sameline;
        nl("0|Clear Buffer|" + cv(NOTIFIER_BUFFER_CLEAR));
        nl("4|Max buffer length (seconds)|" + cv(CVAR_MAX_BUFFER_LENGTH) + "|0|600");
        label("NOTE: File is saved to location specified by \"Special Path\" in the NORMAL RECORDING section. Read the note there for instructions.");
    
        blank;
        separator;
        blank;
    
        label("IMPORTING");
        nl("12|File Name##Import|" + cv(CVAR_IMPORT_FILE_NAME));
        nl("0|Import Selected Animation|" + cv(NOTIFIER_IMPORT_FILE));
        sameline;
        nl("0|Clear Animation|" + cv(NOTIFIER_IMPORT_CLEAR));
    
        blank;
        separator;
        blank;
    
        label("CAMERA OVERRIDES");
        label("NOTE: This may conflict with SpectatorControls. Recommend disabling those overrides while using these.");
        nl2(EUI::Checkbox, CVAR_ENABLE_CAM_OVERRIDE);
        nl2(EUI::GrayedBegin, CVAR_ENABLE_CAM_OVERRIDE);
            sameline;
            nl2(EUI::Checkbox, CVAR_CAM_LOCAL_MOVEMENT);
            sameline;
            nl2(EUI::Checkbox, CVAR_CAM_LOCAL_ROTATION);
            sameline;
            nl2(EUI::Checkbox, CVAR_CAM_HARD_FLOORS);
            sameline;
            nl2(EUI::Checkbox, CVAR_ROLL_REPLACES_PITCH);
            nl2(EUI::GrayedBegin, CVAR_CAM_HARD_FLOORS);
                nl2(EUI::Float, CVAR_CAM_FLOOR_HEIGHT);
            nl2(EUI::GrayedEnd, CVAR_CAM_HARD_FLOORS);
            nl("4|Movement Speed|" + cv(CVAR_CAM_MOVEMENT_SPEED) + "|0|5");
            nl("4|Movement Acceleration|" + cv(CVAR_CAM_MOVEMENT_ACCEL) + "|0|5");
            nl("4|Rotation Speed (doesn't affect mouse)|" + cv(CVAR_ROT_SPEED) + "|0|3");
            nl("4|Rotation Acceleration (Mouse)|" + cv(CVAR_ROT_ACCEL_MOUSE) + "|0|5");
            nl("4|Rotation Acceleration (Controller)|" + cv(CVAR_ROT_ACCEL_GAMEPAD) + "|0|5");
            nl("4|Mouse Sensitivity|" + cv(CVAR_MOUSE_SENSITIVITY) + "|0|25");
            nl("4|Gamepad Sensitivity|" + cv(CVAR_GAMEPAD_SENSITIVITY) + "|0|50");
            nl("4|FOV Rotation Scale|" + cv(CVAR_FOV_ROTATION_SCALE) + "|0|2");
            nl("6|Toggle roll binding|" + cv(CVAR_ROLL_BINDING) + "|" );//+ GetBindingsList());
        nl2(EUI::GrayedEnd, CVAR_ENABLE_CAM_OVERRIDE);

    nl("11|");//!FileIsWriting
    

    SettingsFile.close();
    GlobalCvarManager->executeCommand("cl_settings_refreshplugins");
}
