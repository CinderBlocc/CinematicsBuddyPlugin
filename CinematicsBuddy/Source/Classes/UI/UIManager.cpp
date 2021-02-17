#include "UIManager.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include <filesystem>
#include <fstream>

#define nl(x) SettingsFile << std::string(x) << '\n'
#define blank SettingsFile << '\n'
#define cv(x) std::string(x)

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
    nl("9|Version: " + cv(PLUGIN_VERSION));
    nl("10|" + cv(CVAR_IS_FILE_WRITING));
        nl("10|" + cv(CVAR_IS_FILE_WRITING));
            nl("10|" + cv(CVAR_IS_FILE_WRITING));
                nl("10|" + cv(CVAR_IS_FILE_WRITING));
                    nl("10|" + cv(CVAR_IS_FILE_WRITING));
                        nl("9|File is writing! Please wait for it to finish before continuing.");
                    nl("11|");
                nl("11|");
            nl("11|");
        nl("11|");
    nl("11|");
    
    blank;
    nl("8|");
    blank;

    nl("10|!" + cv(CVAR_IS_FILE_WRITING));

        nl("10|!" + cv(CVAR_IS_RECORDING_ACTIVE));
            nl("9|NORMAL RECORDING");
            nl("1|Automatically increment file names|" + cv(CVAR_INCREMENT_FILES));
            nl("12|File Name##Export|" + cv(CVAR_FILE_NAME));
            nl("12|Camera Name|" + cv(CVAR_CAMERA_NAME));
            nl("1|##UseSpecialPath|" + cv(CVAR_SET_SPECIAL_PATH));
            nl("7|");
            nl("10|" + cv(CVAR_SET_SPECIAL_PATH));
                nl("9|Special Path");
                nl("7|");
                nl("12|##SpecialPath|" + cv(CVAR_SPECIAL_PATH));
                nl("9|NOTE: Unchecking \"Special Path\", or leaving the path textbox blank will use the default /data/CinematicsBuddy/AnimationExports/ folder.");
                nl("9|Recommended setting: leave it disabled and let the plugin handle the paths automatically");
            nl("11|");//SetSpecialPath
            nl("0|Start recording|" + cv(NOTIFIER_RECORD_START));
            nl("7|");
        nl("11|");//IsRecordingActive
        nl("10|" + cv(CVAR_IS_RECORDING_ACTIVE));
            nl("10|!" + cv(CVAR_IS_FILE_WRITING));
                nl("0|Stop recording|" + cv(NOTIFIER_RECORD_STOP));
            nl("11|");//!IsFileWriting
        nl("11|");//IsRecordingActive
        nl("4|Max recording length (seconds)|" + cv(CVAR_MAX_RECORD_LENGTH) + "|0|600");

        blank;
        nl("8|");
        blank;

        nl("9|BUFFER RECORDING");
        nl("1|Enable Buffer|" + cv(CVAR_BUFFER_ENABLED));
        nl("7|");
        nl("0|Capture Buffer|" + cv(NOTIFIER_BUFFER_CAPTURE));
        nl("7|");
        nl("0|Clear Buffer|" + cv(NOTIFIER_BUFFER_CLEAR));
        nl("4|Max buffer length (seconds)|" + cv(CVAR_MAX_BUFFER_LENGTH) + "|0|600");
        nl("9|NOTE: File is saved to location specified by \"Special Path\" in the NORMAL RECORDING section. Read the note there for instructions.");
    
        blank;
        nl("8|");
        blank;
    
        nl("9|IMPORTING");
        nl("12|File Name##Import|" + cv(CVAR_IMPORT_FILE_NAME));
        nl("0|Import Selected Animation|" + cv(NOTIFIER_IMPORT_FILE));
        nl("7|");
        nl("0|Clear Animation|" + cv(NOTIFIER_IMPORT_CLEAR));
    
        blank;
        nl("8|");
        blank;
    
        nl("9|CAMERA OVERRIDES");
        nl("9|NOTE: This may conflict with SpectatorControls. Recommend disabling those overrides while using these.");
        nl("1|Enable Overrides|" + cv(CVAR_ENABLE_CAM_OVERRIDE));
        nl("10|" + cv(CVAR_ENABLE_CAM_OVERRIDE));
            nl("7|");
            nl("1|Use local orientation|" + cv(CVAR_CAM_LOCAL_MATRIX));
            nl("4|Movement Speed|" + cv(CVAR_CAM_MOVEMENT_SPEED) + "|0|5");
            nl("4|Movement Acceleration|" + cv(CVAR_CAM_MOVEMENT_ACCEL) + "|0|5");
            nl("4|Rotation Speed (doesn't affect mouse)|" + cv(CVAR_ROT_SPEED) + "|0|3");
            nl("4|Rotation Acceleration (Mouse)|" + cv(CVAR_ROT_ACCEL_MOUSE) + "|0|5");
            nl("4|Rotation Acceleration (Controller)|" + cv(CVAR_ROT_ACCEL_GAMEPAD) + "|0|5");
            nl("4|Mouse Sensitivity|" + cv(CVAR_MOUSE_SENSITIVITY) + "|0|25");
            nl("4|Gamepad Sensitivity|" + cv(CVAR_GAMEPAD_SENSITIVITY) + "|0|50");
            nl("4|FOV Rotation Scale|" + cv(CVAR_FOV_ROTATION_SCALE) + "|0|2");
            nl("6|Toggle roll binding|" + cv(CVAR_ROLL_BINDING) + "|" + GetBindingsList());
        nl("11|");//Enable overrides

    nl("11|");//!FileIsWriting
    

    SettingsFile.close();
    GlobalCvarManager->executeCommand("cl_settings_refreshplugins");
}

std::string UIManager::GetBindingsList()
{
    static std::string Output;
    static bool bHaveFilledList = false;

    if(!bHaveFilledList)
    {
        //Fill list
        std::vector<std::pair<std::string, std::string>> BindingsList;
        BindingsList.emplace_back("Left thumbstick press", "XboxTypeS_LeftThumbStick");
        BindingsList.emplace_back("Right thumbstick press", "XboxTypeS_RightThumbStick");
        BindingsList.emplace_back("DPad up", "XboxTypeS_DPad_Up");
        BindingsList.emplace_back("DPad left", "XboxTypeS_DPad_Left");
        BindingsList.emplace_back("DPad right", "XboxTypeS_DPad_Right");
        BindingsList.emplace_back("DPad down", "XboxTypeS_DPad_Down");
        BindingsList.emplace_back("Back button", "XboxTypeS_Back");
        BindingsList.emplace_back("Start button", "XboxTypeS_Start");
        BindingsList.emplace_back("Xbox Y - PS4 Triangle", "XboxTypeS_Y");
        BindingsList.emplace_back("Xbox X - PS4 Square", "XboxTypeS_X");
        BindingsList.emplace_back("Xbox B - PS4 Circle", "XboxTypeS_B");
        BindingsList.emplace_back("Xbox A - PS4 X", "XboxTypeS_A");
        BindingsList.emplace_back("Xbox LB - PS4 L1", "XboxTypeS_LeftShoulder");
        BindingsList.emplace_back("Xbox RB - PS4 R1", "XboxTypeS_RightShoulder");
        BindingsList.emplace_back("Xbox LT - PS4 L2", "XboxTypeS_LeftTrigger");
        BindingsList.emplace_back("Xbox RT - PS4 R2", "XboxTypeS_RightTrigger");
        BindingsList.emplace_back("Left thumbstick X axis", "XboxTypeS_LeftX");
        BindingsList.emplace_back("Left thumbstick Y axis", "XboxTypeS_LeftY");
        BindingsList.emplace_back("Right thumbstick X axis", "XboxTypeS_RightX");
        BindingsList.emplace_back("Right thumbstick Y axis", "XboxTypeS_RightY");

        //Compile list into one string
        for(const auto& Binding : BindingsList)
        {
            Output += Binding.first + "@" + Binding.second + "&";
        }

        //Remove last "&"
        Output.pop_back();

        bHaveFilledList = true;
    }

    return Output;
}
