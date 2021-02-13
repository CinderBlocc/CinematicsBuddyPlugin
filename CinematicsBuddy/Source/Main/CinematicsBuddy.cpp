#include "CinematicsBuddy.h"
#include "bakkesmod/wrappers/includes.h"
#include "Misc/CBTimer.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "DataCollectors/FrameInfo.h"
#include "Importing/AnimationImporter.h"
#include "Exporting/AnimationExporter.h"
#include "Exporting/AnimationBuffer.h"
#include <sstream>
#include <fstream>

/*
    @TODO

    - Automatic filename incrementing option
        - Could probably just check if file exists, then append _## and check if new exists. Keep iterating until successful
    - Camera overrides
        - Methods that will need to have custom delta functions made:
            - Local camera movement and rotation
            - Speed control for movement and rotation
        - Input averaging for smoothness
            - Timed buffer using chrono, similar to the deque of RecordedData in AnimationBuffer
    - Write export file asynchronously. Large files will hang up the game for a long time, and even small files cause a hitch
        - Leave UI greyed out until file writing is completed
*/

/*
    FINAL @TODO: Search the solution for "@TODO" and finish all the remaining @TODOs
*/

/*
TO-DO:
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

//0.9.8 - input smoothing and camera speed control
//0.9.9 - import rewrite
//1.0.0 - REMOVE VERSION DEPENDENCIES!!! Completely lock in the formatting of the text in this version so all future updates don't rely on a broken version
//				- Still include version numbers in the file though in case those need to be referenced in troubleshooting

BAKKESMOD_PLUGIN(CinematicsBuddy, "Capture camera, ball, and car animation", PLUGIN_VERSION, PLUGINTYPE_REPLAY)

std::shared_ptr<CVarManagerWrapper> GlobalCvarManager;
std::shared_ptr<GameWrapper>        GlobalGameWrapper;

void CinematicsBuddy::onLoad()
{
    GlobalCvarManager = cvarManager;
    GlobalGameWrapper = gameWrapper;

    Importer = std::make_shared<AnimationImporter>();
    Exporter = std::make_shared<AnimationExporter>();
    Buffer   = std::make_shared<AnimationBuffer>();

    ExportSpecialFilePath = std::make_shared<std::string>("");
	ExportFileName        = std::make_shared<std::string>("");
	ExportCameraName      = std::make_shared<std::string>("");
	ImportFileName        = std::make_shared<std::string>("");
	BufferSize            = std::make_shared<float>(0.f);
	CamSpeed              = std::make_shared<float>(0.f);
	CamRotationSpeed      = std::make_shared<float>(0.f);
	bSetSpecialFilePath   = std::make_shared<bool>(false);
	bIncrementFileNames   = std::make_shared<bool>(false);
	bUseCamOverrides      = std::make_shared<bool>(false);
	bIsRecordingActive    = std::make_shared<bool>(false);
    bIsBufferActive       = std::make_shared<bool>(false);

	GlobalCvarManager->registerCvar(CVAR_SET_SPECIAL_PATH,    "0",   "Enable if you want to use a non-default path", true).bindTo(bSetSpecialFilePath);
	GlobalCvarManager->registerCvar(CVAR_SPECIAL_PATH,        "",    "Set the special export file path. Leave blank for default", true).bindTo(ExportSpecialFilePath);
	GlobalCvarManager->registerCvar(CVAR_INCREMENT_FILES,     "1",   "Automatically append a unique number to file names", true).bindTo(bIncrementFileNames);
	GlobalCvarManager->registerCvar(CVAR_FILE_NAME,           "",    "Set the export file name", true).bindTo(ExportFileName);
	GlobalCvarManager->registerCvar(CVAR_CAMERA_NAME,         "",    "Set the camera name", true).bindTo(ExportCameraName);
	GlobalCvarManager->registerCvar(CVAR_MAX_RECORD_LENGTH,   "300", "Number of seconds to record", true, true, 0, true, 1000).bindTo(BufferSize);
	GlobalCvarManager->registerCvar(CVAR_MAX_BUFFER_LENGTH,   "30",  "Number of seconds to buffer", true, true, 0, true, 1000).bindTo(BufferSize);
	GlobalCvarManager->registerCvar(CVAR_BUFFER_ENABLED,      "0",   "Enable constant recording buffer", false).bindTo(bIsBufferActive);
	GlobalCvarManager->registerCvar(CVAR_IMPORT_FILE_NAME,    "",    "Set the import file name", true).bindTo(ImportFileName);
	GlobalCvarManager->registerCvar(CVAR_ENABLE_CAM_OVERRIDE, "0",   "Enables camera overriding features", true).bindTo(bUseCamOverrides);
	GlobalCvarManager->registerCvar(CVAR_CAM_MOVEMENT_SPEED,  "1",   "Camera movement speed multiplier", true, true, 0, true, 3).bindTo(CamSpeed);
	GlobalCvarManager->registerCvar(CVAR_CAM_ROTATION_SPEED,  "1",   "Camera rotation speed multiplier", true, true, 0, true, 3).bindTo(CamRotationSpeed);
	GlobalCvarManager->registerCvar(CVAR_IS_RECORDING_ACTIVE, "0",   "Display version information on screen", false).bindTo(bIsRecordingActive);
	GlobalCvarManager->registerCvar(CVAR_IS_FILE_WRITING    , "0",   "Handle UI state if file is writing", false, false, 0, false, 0, false);

    GlobalCvarManager->getCvar(CVAR_BUFFER_ENABLED).addOnValueChanged(std::bind(&CinematicsBuddy::OnBufferEnabledChanged, this));
    GlobalCvarManager->getCvar(CVAR_INCREMENT_FILES).addOnValueChanged(std::bind(&CinematicsBuddy::OnIncrementChanged, this));
	
	GlobalCvarManager->registerNotifier(NOTIFIER_RECORD_START,   [this](std::vector<std::string> params){RecordStart();},      "Starts capturing animation data.",              PERMISSION_ALL);
	GlobalCvarManager->registerNotifier(NOTIFIER_RECORD_STOP,    [this](std::vector<std::string> params){RecordStop();},       "Stops capturing animation data",                PERMISSION_ALL);
	GlobalCvarManager->registerNotifier(NOTIFIER_BUFFER_CAPTURE, [this](std::vector<std::string> params){BufferCapture();},    "Captures the data in the buffer",               PERMISSION_ALL);
	GlobalCvarManager->registerNotifier(NOTIFIER_IMPORT_FILE,    [this](std::vector<std::string> params){CamPathImport();},    "Imports a camera animation from a file",        PERMISSION_ALL);
	GlobalCvarManager->registerNotifier(NOTIFIER_IMPORT_CLEAR,   [this](std::vector<std::string> params){CamPathClear();},     "Clears the imported camera animation",          PERMISSION_ALL);
	
    // TESTS - REMOVE WHEN DONE //
    GlobalCvarManager->registerNotifier("CBTestExportFormat", [this](std::vector<std::string> params){TestExportFormat();}, "Prints data from current frame", PERMISSION_ALL);
    GlobalCvarManager->registerNotifier("CBTestPrintFloat", [this](std::vector<std::string> params){TestPrintFloat();}, "Tests the decimal saving PrintFloat function", PERMISSION_ALL);

	GlobalGameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", std::bind(&CinematicsBuddy::RecordingFunction, this));
	GlobalGameWrapper->HookEvent("Function TAGame.PlayerInput_TA.PlayerInput", std::bind(&CinematicsBuddy::PlayerInputTick, this));
	GlobalGameWrapper->HookEvent("Function ProjectX.EngineShare_X.EventPreLoadMap", std::bind(&CinematicsBuddy::OnNewMapLoading, this));

    GenerateSettingsFile();
}
void CinematicsBuddy::onUnload(){}

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

void CinematicsBuddy::TestPrintFloat()
{
    GlobalCvarManager->log(CBUtils::PrintFloat(0.00000019f, 6));
    GlobalCvarManager->log(CBUtils::PrintFloat(0.00000190f, 6));
    GlobalCvarManager->log(CBUtils::PrintFloat(0.00001900f, 6));
    GlobalCvarManager->log(CBUtils::PrintFloat(0.00019000f, 6));
    GlobalCvarManager->log(CBUtils::PrintFloat(0.00190000f, 6));
    GlobalCvarManager->log(CBUtils::PrintFloat(0.01900000f, 6));
    GlobalCvarManager->log(CBUtils::PrintFloat(0.19000000f, 6));
    GlobalCvarManager->log(CBUtils::PrintFloat(1.90000000f, 6));
    GlobalCvarManager->log(CBUtils::PrintFloat(19.0000000f, 6));
}

std::string CinematicsBuddy::GetSpecialFilePath()
{
    return (*bSetSpecialFilePath ? *ExportSpecialFilePath : "");
}

bool CinematicsBuddy::IsValidMode()
{
    return !GlobalGameWrapper->GetCurrentGameState().IsNull();
}



// RECORDING FUNCTION //
void CinematicsBuddy::RecordingFunction()
{
    if(!IsValidMode())
    {
        return;
    }

    //Only get the data for this frame if either Exporter or Buffer are recording
    //Both Exporter and Buffer will only successfully add data if they are recording
    if(Exporter->GetbIsRecording() || Buffer->GetbIsRecording())
    {
        const FrameInfo& ThisFrame = FrameInfo::Get();
        Exporter->AddData(ThisFrame);
        Buffer->AddData(ThisFrame);
    }
}
void CinematicsBuddy::OnNewMapLoading()
{
    //Game is about to switch to a new map and kill the current map
    //In order to get replay metadata, the recording needs to stop here
    Exporter->StopRecording();
}
void CinematicsBuddy::OnIncrementChanged()
{
    Exporter->SetbIncrementFiles(*bIncrementFileNames);
    Buffer->SetbIncrementFiles(*bIncrementFileNames);
}

// NORMAL RECORDING //
void CinematicsBuddy::RecordStart()
{
    Exporter->StartRecording(GetSpecialFilePath(), *ExportFileName, *ExportCameraName);
}
void CinematicsBuddy::RecordStop()
{
    Exporter->StopRecording();
}

// BUFFER RECORDING //
void CinematicsBuddy::BufferCapture()
{
    Buffer->CaptureBuffer(GetSpecialFilePath(), *ExportFileName, *ExportCameraName);
}
void CinematicsBuddy::OnBufferEnabledChanged()
{
    if(*bIsBufferActive)
    {
        Buffer->StartRecording();
    }
    else
    {
        Buffer->StopRecording();
    }
}


/* INPUT OVERRIDE */
void CinematicsBuddy::PlayerInputTick()
{
	if(!gameWrapper->IsInReplay()) return;
	
	if(*bUseCamOverrides)
	{
		/*
		struct CameraMovement
		{
			unsigned char padding[0x160];//352
			float forward;
			float turn;
			float strafe;
			float up;
			float lookup;
		};

		CameraMovement* cm = (CameraMovement*)camInput.memory_address;

		//ControllerInput controls = something.GetInput();
		//Get average of controller inputs
		//assign average to cm->values

		//LOOK AT UPlayerInput_TA which may give better results than this

		cm->forward *= *cvarCamSpeed;
		cm->strafe *= *cvarCamSpeed;
		cm->up *= *cvarCamSpeed;

		cm->turn *= *cvarCamRotationSpeed;
		cm->lookup *= *cvarCamRotationSpeed;
		*/

		PlayerControllerWrapper controller = gameWrapper->GetPlayerController();
		if(controller.IsNull()) return;
		controller.SetSpectatorCameraAccel(4000);//4000 default. Higher = speedier acceleration
		controller.SetSpectatorCameraSpeed(2000);//2000 default.

		/*
		//ALL OF THESE ARE IN PLAYERCONTROLLERWRAPPER
			//Get inputs from this?
			float GetLastInputPitchUp();
			float GetLastInputPitchDown();
			float GetLastInputYawLeft();
			float GetLastInputYawRight();
			float GetLastInputPitch();
			float GetLastInputYaw();

			//Set inputs with this
			void SetAForward(float aForward);
			void SetATurn(float aTurn);
			void SetAStrafe(float aStrafe);
			void SetAUp(float aUp);
			void SetALookUp(float aLookUp);

			//MAYBE try getting inputs from GetAForward() etc, but that likely won't work
		*/
	}
	else
	{
		//Reset values to default and leave player controller alone
		PlayerControllerWrapper controller = gameWrapper->GetPlayerController();
		if(controller.IsNull()) return;
		controller.SetSpectatorCameraAccel(4000);
		controller.SetSpectatorCameraSpeed(2000);
	}
}


/* ANIMATION IMPORT */
bool hasDataVector = false;
bool stopApplyingAnimation = false;
std::vector<std::vector<float>> importDataVector;
void CinematicsBuddy::CamPathImport()
{
    /*
	//Ask user for confirmation if path metadata ID and current replay ID dont match
	if(!gameWrapper->IsInReplay()) return;
	ReplayWrapper replay = gameWrapper->GetGameEventAsReplay().GetReplay();
	if(replay.memory_address == NULL) return;
	std::ifstream inFile(defaultImportPath + "AnimationImports/" + *ImportFileName + ".txt");
	if(!inFile) return;

	importDataVector.clear();
	std::string line;

	getline(inFile, line);//skip "REPLAY METADATA"
	getline(inFile, line);//skip "Name: xyz"
	getline(inFile, line);//get ID line
	std::stringstream IDdelimited(line);
	std::string fileID;
	getline(IDdelimited, fileID, ' ');//skip ID:
	getline(IDdelimited, fileID, ' ');//get ID value
	std::string currentID = replay.GetId().ToString();
			
	if(fileID.compare(currentID) != 0) return;
	
	for(int i=0; i<5; i++)
		getline(inFile, line);//skip the remaining metadata for now

	while(!inFile.eof())
	{
		//Loop through file and store all timestamp and animation data in a float(?) array
		getline(inFile, line);
		replace(line.begin(), line.end(), ',', ' ');
		replace(line.begin(), line.end(), '\t', ' ');
		std::stringstream lineToParse(line);
		std::string value;
		int i=0;
		std::vector<float> tempDataVector;
		while(getline(lineToParse, value, ' '))
		{
			if(!value.empty())
				tempDataVector.push_back(stof(value));
		}
		importDataVector.push_back(tempDataVector);
	}
	
	hasDataVector = true;
	stopApplyingAnimation = false;
	CamPathApply();
    */
}
void CinematicsBuddy::CamPathApply()
{
    /*
	int currentFrame = 0;
	CameraWrapper camera = gameWrapper->GetCamera();
	if(camera.IsNull()) return;
	if (gameWrapper->IsInReplay() && hasDataVector)
	{
		ReplayWrapper replay = gameWrapper->GetGameEventAsReplay().GetReplay();
		if (replay.memory_address != NULL)
		{
			currentFrame = gameWrapper->GetGameEventAsReplay().GetCurrentReplayFrame();

			for(int i=0; i<importDataVector.size(); i++)
			{
				if((int)importDataVector[i][0] == currentFrame)
				{
					float locX = importDataVector[i][1];
					float locY = importDataVector[i][2];
					float locZ = importDataVector[i][3];
					float quatX = importDataVector[i][4];
					float quatY = importDataVector[i][5];
					float quatZ = importDataVector[i][6];
					float quatW = importDataVector[i][7];
					float camFOV = importDataVector[i][8];
					Quat camQuat = {quatX, quatY, quatZ, quatW};
					Rotator tempRot = QuatToRotator(camQuat);
					Rotator newRot = {-tempRot.Pitch, -tempRot.Roll, (tempRot.Yaw + (int)(180*182.044449))};//Pitch, Yaw, Roll reshuffled to fit <-- ??????? what does that even mean

					gameWrapper->GetCamera().SetPOV({{locX, locY, locZ}, newRot, camFOV});
				}
			}	
		}
	}

	if(!stopApplyingAnimation)
		gameWrapper->SetTimeout(std::bind(&CinematicsBuddy::CamPathApply, this), 0.0001f);
    */
}
void CinematicsBuddy::CamPathClear()
{
    /*
	hasDataVector = false;
	stopApplyingAnimation = true;
	importDataVector.clear();
    */
}
