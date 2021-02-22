#include "CameraConfigManager.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "UI/UIManager.h"

CameraConfigManager::CameraConfigManager(std::shared_ptr<UIManager> TheUI)
{
    UI = TheUI;

    //Register cvars
    UI->AddElement({m_CurrentConfig, CVAR_CONFIG_CURRENT,  "Current config",  "Current set of camera settings"                          });
    UI->AddElement({m_NewName,       CVAR_CONFIG_NEW_NAME, "##ConfigNewName", "Name of new config file", -1000001, -1000001, true, false});

    //Bind addOnValueChanged functions to all cvars from CameraManager and InputsManager
    for(const auto& Cvar : GetCvarList())
    {
        ON_CVAR_CHANGED(Cvar, CameraConfigManager::ResetManager);
    }

    //Update the list of available configs
    UpdateConfigList();

    //Bind addOnValueChanged function to apply config when selection changes
    ON_CVAR_CHANGED(CVAR_CONFIG_CURRENT, CameraConfigManager::ApplyConfig);
    ApplyConfig();

    //Register notifiers
    MAKE_NOTIFIER(NOTIFIER_CONFIG_SAVE,   SaveConfig,       "Save the current config");
    MAKE_NOTIFIER(NOTIFIER_CONFIG_UPDATE, UpdateConfigList, "Update the list of configs");
}

std::vector<std::string> CameraConfigManager::GetCvarList()
{
    static std::vector<std::string> Output;
    static bool bHaveFilledList = false;

    if(!bHaveFilledList)
    {
        //CameraManager cvars
        Output.emplace_back(CVAR_CAM_LOCAL_MOVEMENT);
        Output.emplace_back(CVAR_CAM_LOCAL_ROTATION);
        Output.emplace_back(CVAR_CAM_HARD_FLOORS);
        Output.emplace_back(CVAR_CAM_FLOOR_HEIGHT);
        Output.emplace_back(CVAR_CAM_MOVEMENT_SPEED);
        Output.emplace_back(CVAR_CAM_MOVEMENT_ACCEL);
        Output.emplace_back(CVAR_ROT_SPEED);
        Output.emplace_back(CVAR_ROT_ACCEL_MOUSE);
        Output.emplace_back(CVAR_ROT_ACCEL_GAMEPAD);
        Output.emplace_back(CVAR_MOUSE_SENSITIVITY);
        Output.emplace_back(CVAR_GAMEPAD_SENSITIVITY);
        Output.emplace_back(CVAR_FOV_ROTATION_SCALE);

        //InputsManager cvars
        Output.emplace_back(CVAR_ROLL_BINDING);
        Output.emplace_back(CVAR_FOV_BINDING);
        Output.emplace_back(CVAR_ROLL_SWAP);
        Output.emplace_back(CVAR_FOV_SWAP);
        Output.emplace_back(CVAR_INVERT_PITCH);

        bHaveFilledList = true;
    }

    return Output;
}

void CameraConfigManager::ResetManager()
{
    GlobalCvarManager->getCvar(CVAR_CONFIG_CURRENT).setValue("");
}

void CameraConfigManager::ApplyConfig()
{
    //#TODO: Read selected config starting from the root CameraConfigs folder
}

void CameraConfigManager::SaveConfig()
{
    //#TODO: Write all cvars from GetCvarList to the file specified by CVAR_CONFIG_NEW_NAME
}

void CameraConfigManager::UpdateConfigList()
{
    UIElement::DropdownOptionsType ConfigsList;
    ConfigsList.emplace_back("", "");

    //#TODO: Recursively go through all folders in CameraConfigs and add their relative path to the list
    //  i.e. Subfolder/Filename

    UI->EditElement(CVAR_CONFIG_CURRENT).AddDropdownOptions(ConfigsList);
}
