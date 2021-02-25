#include "CameraConfigManager.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "UI/UIManager.h"
#include <fstream>

/*

    #TODO:

        - Make sure you have all the relevant cvars included here before release

*/

CameraConfigManager::CameraConfigManager(std::shared_ptr<UIManager> TheUI)
{
    UI = TheUI;

    //Register cvars
    UI->AddElement({m_CurrentConfig, CVAR_CONFIG_CURRENT,  "Current config",  "Current set of camera settings"                          });
    UI->AddElement({m_NewName,       CVAR_CONFIG_NEW_NAME, "New config name", "Name of new config file", -1000001, -1000001, true, false});

    //Bind addOnValueChanged functions to all cvars from CameraManager and InputsManager
    for(const auto& Cvar : GetCvarList())
    {
        ON_CVAR_CHANGED(Cvar, CameraConfigManager::ResetManager);
    }

    //Update the list of available configs
    UpdateConfigList(false);

    //Bind addOnValueChanged function to apply config when selection changes
    ON_CVAR_CHANGED(CVAR_CONFIG_CURRENT, CameraConfigManager::ApplyConfig);
    ApplyConfig();

    //Register notifiers
    MAKE_NOTIFIER(NOTIFIER_CONFIG_UPDATE, UpdateConfigList, "Update the list of configs");
    MAKE_NOTIFIER(NOTIFIER_CONFIG_SAVE,   SaveConfig,       "Save the current config");
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
        Output.emplace_back(CVAR_ROT_SPEED_MOUSE);
        Output.emplace_back(CVAR_ROT_SPEED_GAMEPAD);
        Output.emplace_back(CVAR_ROT_ACCEL_MOUSE);
        Output.emplace_back(CVAR_ROT_ACCEL_GAMEPAD);
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
    if(!bApplyingConfig)
    {
        GlobalCvarManager->getCvar(CVAR_CONFIG_CURRENT).setValue("");
    }
}

void CameraConfigManager::ApplyConfig()
{
    std::filesystem::path ChosenFile = GetConfigsFolder() / (*m_CurrentConfig + EXTENSION_CONFIG);
    if(!std::filesystem::exists(ChosenFile))
    {
        return;
    }

    std::ifstream InFile(ChosenFile);
    if(!InFile.is_open())
    {
        return;
    }

    GlobalCvarManager->log("Applying config: " + GetRelativeFilename(ChosenFile));

    bApplyingConfig = true;
    while(!InFile.eof())
    {
        std::string Line;
        getline(InFile, Line);
        
        if(Line.empty())
        {
            continue;
        }

        GlobalCvarManager->executeCommand(Line, false);
    }
    bApplyingConfig = false;

    InFile.close();
}

void CameraConfigManager::SaveConfig()
{
    if(m_NewName->empty())
    {
        return;
    }

    std::filesystem::path NewFile = GetConfigsFolder() / (*m_NewName + EXTENSION_CONFIG);
    std::ofstream OutFile(NewFile);
    if(!OutFile.is_open())
    {
        return;
    }

    for(const auto& CvarName : GetCvarList())
    {
        CVarWrapper TheCvar = GlobalCvarManager->getCvar(CvarName);
        if(!TheCvar.IsNull())
        {
            OutFile << CvarName << " " << TheCvar.getStringValue() << '\n';
        }
    }

    OutFile.close();

    UpdateConfigList();
}

void CameraConfigManager::UpdateConfigList(bool bRefresh)
{
    UIElement::DropdownOptionsType ConfigsList;
    ConfigsList.emplace_back("", "");

    for(const auto& TheFile : std::filesystem::recursive_directory_iterator(GetConfigsFolder()))
    {
        if(TheFile.path().extension().u8string() == EXTENSION_CONFIG)
        {
            std::string FinalFileName = GetRelativeFilename(TheFile);
            ConfigsList.emplace_back(FinalFileName, FinalFileName);
        }
    }

    UI->EditElement(CVAR_CONFIG_CURRENT).AddDropdownOptions(ConfigsList);

    //Don't refresh during construction, only when manually calling by notifier
    if(bRefresh)
    {
        UI->GenerateSettingsFile();
    }
}

std::filesystem::path CameraConfigManager::GetConfigsFolder()
{
    std::filesystem::path ConfigFolder = GlobalGameWrapper->GetDataFolder() / "CinematicsBuddy" / "CameraConfigs";
    if(!std::filesystem::exists(ConfigFolder))
    {
        std::filesystem::create_directory(ConfigFolder);
    }

    return ConfigFolder;
}

std::string CameraConfigManager::GetRelativeFilename(const std::filesystem::path& InPath)
{
    auto TheFile = std::filesystem::proximate(InPath, GetConfigsFolder());
    std::string FileName = TheFile.string();
    std::string FinalFileName = FileName.substr(0, FileName.size() - 4);

    return FinalFileName;
}
