#include "UIManager.h"
#include "UIElement.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include <filesystem>
#include <fstream>

UIManager* UIManager::Instance_;

UIManager* UIManager::GetInstance()
{
    if(!Instance_)
    {
        Instance_ = new UIManager();
    }

    return Instance_;
}

void UIManager::DestroyInstance()
{
    //Refer to CinematicsBuddy::onUnload for explanation
    if(Instance_)
    {
        delete Instance_;
        Instance_ = nullptr;
    }
}

#define nl(x) SettingsFile << std::string(x) << '\n'
#define nl2(x, y) nl(m_Elements[y].Print(x))
#define nl3(x, y, z) nl(m_Elements[y].Print(x, z))
#define cv(x) std::string(x)
#define blank SettingsFile << '\n'
#define sameline nl("7|")
#define separator nl("8|")
#define label(x) nl(std::string("9|") + x)
#define button(x, y) nl(std::string("0|") + x + "|" + y)

void UIManager::GenerateSettingsFile()
{
    //Get the settings file directory
    std::filesystem::path SettingsFilePath = GlobalGameWrapper->GetBakkesModPath() / "plugins" / "settings";
    if(!std::filesystem::exists(SettingsFilePath))
    {
        GlobalCvarManager->log("ERROR: Failed to write settings file. Path (" + SettingsFilePath.string() + ") does not exist");
        return;
    }

    //Open the settings file
    std::ofstream SettingsFile(SettingsFilePath / "CinematicsBuddy.set");
    if(!SettingsFile.is_open())
    {
        GlobalCvarManager->log("ERROR: Failed to write settings file. File could not be opened");
        return;
    }


    //////////// BEGIN WRITING SETTINGS FILE ////////////


    //Header
    nl("Cinematics Buddy");
    label("Version: " + cv(PLUGIN_VERSION));

    //Multiple nested depth grayed components to really hide this warning label
    constexpr int WarningDepth = 5;
    for(int i = 0; i < WarningDepth; ++i) { nl2(EUI::GrayedBegin, CVAR_IS_FILE_WRITING); }
        sameline;
        label(" -- File is writing! Please wait for it to finish before continuing --");
    for(int i = 0; i < WarningDepth; ++i) { nl2(EUI::GrayedEnd, CVAR_IS_FILE_WRITING); }
    
    blank;
    separator;
    blank;

#ifndef IS_GAMMA_BUILD
    nl3(EUI::GrayedBegin, CVAR_IS_FILE_WRITING, true);

        //Normal recording
        nl3(EUI::GrayedBegin, CVAR_IS_RECORDING_ACTIVE, true);
            label("NORMAL RECORDING");
            nl2(EUI::Checkbox, CVAR_INCREMENT_FILES);
            nl2(EUI::Textbox, CVAR_FILE_NAME);
            nl2(EUI::Textbox, CVAR_CAMERA_NAME);
            nl2(EUI::Checkbox, CVAR_SET_SPECIAL_PATH);
            sameline;
            nl2(EUI::GrayedBegin, CVAR_SET_SPECIAL_PATH);
                label("Special Path");
                sameline;
                nl2(EUI::Textbox, CVAR_SPECIAL_PATH);
                label("NOTE: Unchecking \"Special Path\", or leaving the path textbox blank will use the default /data/CinematicsBuddy/AnimationExports/ folder.");
                label("Recommended setting: leave it disabled and let the plugin handle the paths automatically");
            nl2(EUI::GrayedEnd, CVAR_SET_SPECIAL_PATH);
            button("Start Recording", NOTIFIER_RECORD_START);
            sameline;
        nl2(EUI::GrayedEnd, CVAR_IS_RECORDING_ACTIVE);
        nl2(EUI::GrayedBegin, CVAR_IS_RECORDING_ACTIVE);
            nl3(EUI::GrayedBegin, CVAR_IS_FILE_WRITING, true);
                button("Stop Recording", NOTIFIER_RECORD_STOP);
            nl2(EUI::GrayedEnd, CVAR_IS_FILE_WRITING);
        nl2(EUI::GrayedEnd, CVAR_IS_RECORDING_ACTIVE);
        nl2(EUI::Float, CVAR_MAX_RECORD_LENGTH);

        blank;
        separator;
        blank;

        //Buffer recording
        label("BUFFER RECORDING");
        nl2(EUI::Checkbox, CVAR_BUFFER_ENABLED);
        sameline;
        button("Capture Buffer", NOTIFIER_BUFFER_CAPTURE);
        sameline;
        button("Clear Buffer", NOTIFIER_BUFFER_CLEAR);
        nl2(EUI::Float, CVAR_MAX_BUFFER_LENGTH);
        label("NOTE: File is saved to location specified by \"Special Path\" in the NORMAL RECORDING section. Read the note there for instructions.");
    
        blank;
        separator;
        blank;
    
        //Importing camera animations
        label("IMPORTING");
        nl2(EUI::Textbox, CVAR_IMPORT_FILE_NAME);
        button("Import Selected Animation", NOTIFIER_IMPORT_FILE);
        sameline;
        button("Clear Animation", NOTIFIER_IMPORT_CLEAR);

    nl2(EUI::GrayedEnd, CVAR_IS_FILE_WRITING);
#endif
#ifdef IS_GAMMA_BUILD
    label("Recording, Buffer, and Importing are unavailable in Beta/Gamma builds");
#endif
    
    blank;
    separator;
    blank;
    
    //Camera input overrides
    label("CAMERA OVERRIDES");
    label("NOTE: This may conflict with SpectatorControls. Recommend disabling those overrides while using these.");
    nl2(EUI::Checkbox, CVAR_ENABLE_CAM_OVERRIDE);
    nl2(EUI::GrayedBegin, CVAR_ENABLE_CAM_OVERRIDE);
        sameline;
        nl2(EUI::Checkbox, CVAR_CAM_LOCAL_MOMENTUM);
        sameline;
        nl2(EUI::Checkbox, CVAR_CAM_LOCAL_MOVEMENT);
        sameline;
        nl2(EUI::Checkbox, CVAR_CAM_LOCAL_ROTATION);
        sameline;
        nl2(EUI::Checkbox, CVAR_INVERT_PITCH);
        sameline;
        nl2(EUI::Checkbox, CVAR_CAM_HARD_FLOORS);
        nl2(EUI::GrayedBegin, CVAR_CAM_HARD_FLOORS);
            nl2(EUI::Float, CVAR_CAM_FLOOR_HEIGHT);
        nl2(EUI::GrayedEnd, CVAR_CAM_HARD_FLOORS);
        nl2(EUI::Float, CVAR_CAM_MOVEMENT_SPEED);
        nl2(EUI::Float, CVAR_CAM_MOVEMENT_ACCEL);
        nl2(EUI::Float, CVAR_ROT_SPEED_MOUSE);
        nl2(EUI::Float, CVAR_ROT_SPEED_GAMEPAD);
        nl2(EUI::Float, CVAR_ROT_ACCEL_MOUSE);
        nl2(EUI::Float, CVAR_ROT_ACCEL_GAMEPAD);
        nl2(EUI::Float, CVAR_FOV_ROTATION_SCALE);
        nl2(EUI::Float, CVAR_FOV_MIN);
        nl2(EUI::Float, CVAR_FOV_MAX);
        nl2(EUI::Float, CVAR_FOV_SPEED);
        nl2(EUI::Float, CVAR_FOV_ACCELERATION);
        nl2(EUI::Float, CVAR_FOV_LIMIT_EASE);
        nl2(EUI::Dropdown, CVAR_ROLL_BINDING);
        nl2(EUI::Dropdown, CVAR_ROLL_SWAP);
        nl2(EUI::Dropdown, CVAR_FOV_BINDING);
        nl2(EUI::Dropdown, CVAR_FOV_SWAP);
        label("CONFIGS");
        nl2(EUI::Dropdown, CVAR_CONFIG_CURRENT);
        sameline;
        button("Update Config List", NOTIFIER_CONFIG_UPDATE);
        nl2(EUI::Textbox, CVAR_CONFIG_NEW_NAME);
        sameline;
        button("Save Config", NOTIFIER_CONFIG_SAVE);
    nl2(EUI::GrayedEnd, CVAR_ENABLE_CAM_OVERRIDE);
    

    //////////// END WRITING SETTINGS FILE ////////////


    SettingsFile.close();
    GlobalCvarManager->executeCommand("cl_settings_refreshplugins");
}
