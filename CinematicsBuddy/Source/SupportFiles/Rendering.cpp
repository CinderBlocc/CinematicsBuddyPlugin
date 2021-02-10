#include "Main/CinematicsBuddy.h"
#include "SupportFiles/MacrosStructsEnums.h"

void CinematicsBuddy::Render(CanvasWrapper canvas)
{
	if(bRecording) return;
	if(gameWrapper->IsInOnlineGame()) return;

	std::vector<std::string> drawStrings;

	if(*bShowVersionInfo)
	{
		drawStrings.push_back("CinematicsBuddy version: " + std::string(PLUGIN_VERSION));
	}

	for(const auto& WarningString : WarningStrings)
    {
		drawStrings.push_back(WarningString);
    }


    canvas.SetColor(LinearColor{0,255,0,255});
	Vector2 base = {50,50};
	for(const auto& ThisString : drawStrings)
	{
		canvas.SetPosition(base);
		canvas.DrawString(ThisString);
		base.Y += 20;
	}
}
