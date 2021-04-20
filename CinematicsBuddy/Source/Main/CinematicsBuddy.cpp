#include "CinematicsBuddy.h"
#include "bakkesmod/wrappers/includes.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "DataCollectors/FrameInfo.h"
#include "Importing/AnimationImporter.h"
#include "Exporting/AnimationExporter.h"
#include "Exporting/AnimationBuffer.h"
#include "Camera/CameraManager.h"
#include "UI/UIManager.h"

/*
    #TODO

    - Make notifier to kill camera momentum
        - Name that "Freeze" and rename the current Freeze to "Block Inputs"

    - Make "BetaCleanup" class

    BETA FILES TO DELETE:                                       << Compare against "_Beta_cleanup" folder in /_EXTRA/Media/CinematicsBuddy/Extra/ one you've updated everything in _MAIN_REPO
        - /plugins/settings/cinematicsbuddy0.9.4c.set
        - /data/CinematicsBuddy/Plugins/3dsMax/CinematicsBuddyMaxscript0.9.4c.ms
        - /data/CinematicsBuddy/Plugins/3dsMax/Assets/      <<< DELETE THE ENTIRE FOLDER

    - Add a checkbox to save dollycam path with the same name as the CinematicsBuddy file

    - Add a README with instructions in these folders:
        - /data/CinematicsBuddy/ for overall instructions about the plugin
        - /"/"/Plugins/3dsMax/ for instructions about maxscript
        - /"/"/Plugins/AfterEffects/ for instructions about AE script

    - Camera animation importing
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

/*
    FINAL #TODO: Look for all TODO tags in VA Hashtags and finish them. Also look for all "TESTS - REMOVE WHEN DONE" and remove them.
*/

BAKKESMOD_PLUGIN(CinematicsBuddy, "Capture camera, ball, and car animation", PLUGIN_VERSION, PLUGINTYPE_REPLAY)

std::shared_ptr<CVarManagerWrapper> GlobalCvarManager;
std::shared_ptr<GameWrapper>        GlobalGameWrapper;

void CinematicsBuddy::onLoad()
{
    //Assign these so any file that includes MacrosStructsEnums can use them
    GlobalCvarManager = cvarManager;
    GlobalGameWrapper = gameWrapper;

    //Initialize each class. Check their constructors for cvars and notifiers
    UI       = std::make_shared<UIManager>();
    Importer = std::make_shared<AnimationImporter>(UI);
    Exporter = std::make_shared<AnimationExporter>(UI);
    Buffer   = std::make_shared<AnimationBuffer>(UI);
    Camera   = std::make_shared<CameraManager>(UI);
	
    //Hook viewport tick for both recording the animation per tick, and applying imported animation per tick
    GlobalGameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", std::bind(&CinematicsBuddy::OnViewportTick, this));

    //Dynamically generate the UI
    UI->GenerateSettingsFile();
}
void CinematicsBuddy::onUnload(){}


// TICK FUNCTIONS //
void CinematicsBuddy::OnViewportTick()
{
    //Apply imported animation
    Importer->ApplyCameraAnimation();
    
    //Record animations
    //Should come after Importer's animation so that recordings aren't a tick late
    RecordingFunction();
}

bool CinematicsBuddy::IsValidRecordingMode()
{
    if(GlobalGameWrapper->GetCurrentGameState().IsNull())
    {
        return false;
    }

    if(!Exporter->GetbIsRecording() && !Buffer->GetbIsRecording())
    {
        return false;
    }

    return true;
}

void CinematicsBuddy::RecordingFunction()
{
    if(IsValidRecordingMode())
    {
        FrameInfo ThisFrame = FrameInfo::Get();
        Exporter->AddData(ThisFrame);
        Buffer->AddData(ThisFrame);
    }
}
