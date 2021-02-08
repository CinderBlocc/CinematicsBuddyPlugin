#include "Main/CinematicsBuddy.h"
#include <fstream>

#define nl(x) SettingsFile << std::string(x) << '\n'
#define blank SettingsFile << '\n'
#define cv(x) std::string(x)

void CinematicsBuddy::GenerateSettingsFile()
{
    std::ofstream SettingsFile(gameWrapper->GetBakkesModPath() / "plugins" / "settings" / "CinematicsBuddy.set");

    nl("Cinematics Buddy");
    blank;
    

    SettingsFile.close();
    cvarManager->executeCommand("cl_settings_refreshplugins");
}
