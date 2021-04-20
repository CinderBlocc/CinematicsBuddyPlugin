#include "BetaCleanup.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"

void BetaCleanup::RemoveBetaFiles()
{
    // -- /plugins/settings/cinematicsbuddy0.9.4c.set
    auto OldSettingsFile = GlobalGameWrapper->GetBakkesModPath() / "plugins" / "settings" / "cinematicsbuddy0.9.4c.set";
    
    // -- /data/CinematicsBuddy/Plugins/3dsMax/CinematicsBuddyMaxscript0.9.4c.ms
    // -- /data/CinematicsBuddy/Plugins/3dsMax/Assets/
    auto MaxFolder = GlobalGameWrapper->GetDataFolder() / "CinematicsBuddy" / "Plugins" / "3dsMax";
    auto OldMaxscriptFile = MaxFolder / "CinematicsBuddyMaxscript0.9.4c.ms";
    auto OldMaxAssetsFolder = MaxFolder / "Assets";

    // Delete all
    RemoveFile(OldSettingsFile);
    RemoveFile(OldMaxscriptFile);
    RemoveFolder(OldMaxAssetsFolder);
}

void BetaCleanup::RemoveFile(std::filesystem::path TheFile)
{
    if(std::filesystem::exists(TheFile))
    {
        std::filesystem::remove(TheFile);
        GlobalCvarManager->log("Removed beta file: \"" + TheFile.string() + "\"");
    }
}

void BetaCleanup::RemoveFolder(std::filesystem::path TheFolder)
{
    if(std::filesystem::exists(TheFolder))
    {
        std::filesystem::remove_all(TheFolder);
        GlobalCvarManager->log("Removed beta folder along with all of its contents: \"" + TheFolder.string() + "\"");
    }
}
