#include "CinematicsBuddy.h"
#include "bakkesmod/wrappers/includes.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "SupportFiles/BetaCleanup.h"
#include "DataCollectors/FrameInfo.h"
#include "Importing/AnimationImporter.h"
#include "Exporting/AnimationExporter.h"
#include "Exporting/AnimationBuffer.h"
#include "Camera/CameraManager.h"
#include "UI/UIManager.h"

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

    //Clean out the old beta files to avoid confusion for the end user
    //Eventually this should be deleted
    BetaCleanup::RemoveBetaFiles();
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
