#include "AnimationImporter.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "UI/UIManager.h"

/*
    #TODO: Implement this

    AnimationImporter was originally planned to be released in 1.0, but due to time restrictions that won't be happening.
    It will be added in a future version, time willing.

    NOTES:
        - IMPORT INTERPOLATION
			- https://discordapp.com/channels/@me/602523400518369280/676118235631845398
			- https://discordapp.com/channels/@me/602523400518369280/700841996927107162
		- Reset SeqAct animations for world items so all recordings will line up perfectly
			- from glhglh and Martinn: gameWrapper->ExecuteUnrealCommand("SET SeqAct_Interp Position 0.0");
		- Animation while the replay is paused
			- This would just be an output of a list of frames not tied to any particular replay timestamp
				- Maybe it should have an output of the timestamp it was saved at so it can go to that exact replay frame
			- Still use the chrono delta for these frames
*/

AnimationImporter::AnimationImporter(std::shared_ptr<UIManager> TheUI)
{
    UI = TheUI;

#ifdef INCLUDE_ANIMATION_IMPORTING
    //Register cvars
    UI->AddElement({ImportFileName, CVAR_IMPORT_FILE_NAME, "File Name##Import", "Set the import file name"});

    //Register notifiers
    MAKE_NOTIFIER(NOTIFIER_IMPORT_FILE,  ImportCameraAnimation, "Imports a camera animation from a file");
	MAKE_NOTIFIER(NOTIFIER_IMPORT_CLEAR, ClearCameraAnimation,  "Clears the imported camera animation");
#endif
}

void AnimationImporter::ImportCameraAnimation()
{
    //Import the file specified by *ImportFileName
}

void AnimationImporter::ApplyCameraAnimation()
{
    //Called per tick by plugin. Only apply if in valid time range and if bPlayAnimation is true
}

void AnimationImporter::ClearCameraAnimation()
{
    //Clear the cached data and set the bPlayAnimation flag to false
}
