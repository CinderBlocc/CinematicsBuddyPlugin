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

    - Write export file asynchronously. Large files will hang up the game for a long time, and even small files cause a hitch
        - Leave UI greyed out until file writing is completed
    - Write all class members with m_ prefix? Either that or remove the prefix from CameraManager's members


    MAXSCRIPT NOTES:
        - When attempting to export FBX to other package, parenting might not export correctly.
            - In the script (button in UI), add an option to bake the animation.
            - Create a clone of the car mesh, then bake all of the position/rotation data onto the mesh.
            - Add all the cloned cars to the selection, along with the field, ball, and camera. Export selected to FBX.
                - Let the user choose the FBX settings, so make sure that dialog pops up


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

    - Remove BMGraphs as a submodule
    - Extract what you need from Matrix3 and then remove RenderingTools as a submodule?
*/

/*
    FINAL #TODO: Look for all TODO tags in VA Hashtags and finish them
                 Also look for all "TESTS" and remove them
*/

//0.9.8 - input smoothing and camera speed control
//0.9.9 - import rewrite
//1.0.0 - REMOVE VERSION DEPENDENCIES!!! Completely lock in the formatting of the text in this version so all future updates don't rely on a broken version
//				- Still include version numbers in the file though in case those need to be referenced in troubleshooting

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

    // TESTS - REMOVE WHEN DONE //
    GlobalCvarManager->registerNotifier("CBTestExportFormat", [this](std::vector<std::string> params){TestExportFormat();}, "Prints data from current frame", PERMISSION_ALL);
    GlobalGameWrapper->RegisterDrawable(std::bind(&CinematicsBuddy::DebugRender, this, std::placeholders::_1));
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


// TESTS - REMOVE WHEN DONE //
void CinematicsBuddy::TestExportFormat()
{
    //Gets the info of the current frame, and prints it in its final format to the console
    FrameInfo ThisFrame = FrameInfo::Get();
    const auto& ThisFrameTime = ThisFrame.GetTimeInfo();
    const auto& CarsSeenThisFrame = ThisFrame.GetCarsSeen();
    
    std::string Output;
    Output += "\n\nEXAMPLE FORMAT\n" + FrameInfo::PrintExampleFormat();
    Output += "\n\nTHIS FRAME\n" + ThisFrame.Print(ThisFrameTime, 0, CarsSeenThisFrame);

    GlobalCvarManager->log(Output);
}
void CinematicsBuddy::DebugRender(CanvasWrapper Canvas)
{
    Camera->DebugRender(Canvas);
}
