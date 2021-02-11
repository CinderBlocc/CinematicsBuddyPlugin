#include "CinematicsBuddy.h"
#include "bakkesmod/wrappers/includes.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "Classes/DataCollectors/FrameInfo.h"
#include "Classes/Importing/AnimationImporter.h"
#include "Classes/Exporting/AnimationExporter.h"
#include "Classes/Exporting/AnimationBuffer.h"
#include <sstream>
#include <fstream>

using namespace std::chrono;

/*
    NEW @TODO

    - If the file path is blank, use the default Import/Export folders
        - Grey out the text box and have a checkbox to enable it so they need to read instructions before applying an unnecessary path
    - Stop the recording on Soccar_TA destroyed to ensure only one replay gets recorded in a file

*/


/*
	PRIORITY

	- File incrementation option
*/

/*
TO-DO:
	- LOCAL CAMERA MOVEMENT: As soon as the camera has rolled in flycam, it's gg trying to control it anymore
		- Add an option to override movement so that inputs will be relative to camera's rotation
		- i.e. holding trigger will move camera along local up axis instead of world up axis
		- i.e. pitch and yaw will be local instead of global after camera has rolled

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

	- Camera smoothing (use bakkes' CameraInputModifier for reference _Youtube/4_extraassets/bakkesmod_extra/CameraInputModifier)
		- Look through previous CB versions. How did you edit player input without having the input constantly multiply over itself?
		- Try to find the speed limiter in the full SDK and uncap the max speed for the camera movement
		- BETTER OPTION
			- Fill buffer with inputs. Delete inputs if their capture time is outside the smoothing time
				struct InputVal
				{
					chrono::clock timeOfCapture
					CameraInput capturedInputs;
				}
				while((chrono::now() - inputVals[0].timeOfCapture) > smoothingTime) inputVals.erase(inputVals.begin());
			- Get average of all inputs. Use that average as the assigned input.
		- Easing: https://forum.unity.com/threads/joystick-easing-script.11535/   //   https://answers.unrealengine.com/questions/726110/how-to-ease-the-player-controller-rotation-speed.html

	- Export car mesh animation instead of car rigid body animation?
		- This will make the car align better in external software
		- Do the same for the ball mesh since it doesn't seem to line up very well at times either
*/

//0.9.7 - export rewrite
//0.9.8 - input smoothing and camera speed control
//0.9.9 - import rewrite
//1.0.0 - car mesh export? Definitely wrap things up like error rendering before 1.0.0
//		- REMOVE VERSION DEPENDENCIES IN 1.0.0!!! Completely lock in the formatting of the text file before version 1 so all future updates dont rely on a broken version
//				- Still include version numbers in the file though in case those need to be referenced in troubleshooting

BAKKESMOD_PLUGIN(CinematicsBuddy, "Cinematics Buddy Plugin", PLUGIN_VERSION, PLUGINTYPE_REPLAY)

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
	bShowVersionInfo      = std::make_shared<bool>(false);
	bUseCamVelocity       = std::make_shared<bool>(false);
	bSetSpecialFilePath   = std::make_shared<bool>(false);

	cvarManager->registerCvar(CVAR_SET_SPECIAL_PATH,   "0",   "Enable if you want to use a non-default path", true).bindTo(bSetSpecialFilePath);
	cvarManager->registerCvar(CVAR_SPECIAL_PATH,       "",    "Set the special export file path. Leave blank for default", true).bindTo(ExportSpecialFilePath);
	cvarManager->registerCvar(CVAR_FILE_NAME,          "",    "Set the export file name", true).bindTo(ExportFileName);
	cvarManager->registerCvar(CVAR_CAMERA_NAME,        "",    "Set the camera name", true).bindTo(ExportCameraName);
	cvarManager->registerCvar(CVAR_MAX_RECORD_LENGTH,  "300", "Number of seconds to record", true, true, 0, true, 1000).bindTo(BufferSize);
	cvarManager->registerCvar(CVAR_MAX_BUFFER_LENGTH,  "30",  "Number of seconds to buffer", true, true, 0, true, 1000).bindTo(BufferSize);
	cvarManager->registerCvar(CVAR_IMPORT_FILE_NAME,   "",    "Set the import file name", true).bindTo(ImportFileName);
	cvarManager->registerCvar(CVAR_SMOOTH_CAM_INPUTS,  "0",   "Smooth camera movements", true).bindTo(bUseCamVelocity);
	cvarManager->registerCvar(CVAR_CAM_MOVEMENT_SPEED, "1",   "Camera movement speed multiplier", true, true, 0, true, 3).bindTo(CamSpeed);
	cvarManager->registerCvar(CVAR_CAM_ROTATION_SPEED, "1",   "Camera rotation speed multiplier", true, true, 0, true, 3).bindTo(CamRotationSpeed);
	cvarManager->registerCvar(CVAR_SHOW_VERSION_INFO,  "0",   "Display version information on screen", true).bindTo(bShowVersionInfo);
	
	cvarManager->registerNotifier(NOTIFIER_RECORD_START,   [this](std::vector<std::string> params){RecordStart();},      "Starts capturing animation data.",              PERMISSION_ALL);
	cvarManager->registerNotifier(NOTIFIER_RECORD_STOP,    [this](std::vector<std::string> params){RecordStop();},       "Stops capturing animation data",                PERMISSION_ALL);
	cvarManager->registerNotifier(NOTIFIER_BUFFER_START,   [this](std::vector<std::string> params){BufferStart();},      "Starts the perpetual animation capture buffer", PERMISSION_ALL);
	cvarManager->registerNotifier(NOTIFIER_BUFFER_CAPTURE, [this](std::vector<std::string> params){BufferCapture();},    "Captures the data in the buffer",               PERMISSION_ALL);
	cvarManager->registerNotifier(NOTIFIER_BUFFER_STOP,    [this](std::vector<std::string> params){BufferCancel();},     "Cancels the perpetual animation buffer",        PERMISSION_ALL);
	cvarManager->registerNotifier(NOTIFIER_IMPORT_FILE,    [this](std::vector<std::string> params){CamPathImport();},    "Imports a camera animation from a file",        PERMISSION_ALL);
	cvarManager->registerNotifier(NOTIFIER_IMPORT_CLEAR,   [this](std::vector<std::string> params){CamPathClear();},     "Clears the imported camera animation",          PERMISSION_ALL);
	cvarManager->registerNotifier(NOTIFIER_TEST_EXPORT,    [this](std::vector<std::string> params){TestExportFormat();}, "Prints data from current frame",                PERMISSION_ALL);

	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", std::bind(&CinematicsBuddy::RecordingFunction, this));
	gameWrapper->HookEvent("Function TAGame.PlayerInput_TA.PlayerInput", std::bind(&CinematicsBuddy::PlayerInputTick, this));

    GenerateSettingsFile();
}
void CinematicsBuddy::onUnload(){}

void CinematicsBuddy::TestExportFormat()
{
    //Gets the info of the current frame, and prints it in its final format to the console
    FrameInfo ThisFrame = FrameInfo::Get();
    const auto& ThisFrameTime = ThisFrame.GetTimeInfo();
    const auto& CarsSeenThisFrame = ThisFrame.GetCarsSeen();
    GlobalCvarManager->log("\n" + ThisFrame.Print(ThisFrameTime, 0, CarsSeenThisFrame));
}

std::string CinematicsBuddy::GetSpecialFilePath()
{
    return (*bSetSpecialFilePath ? *ExportSpecialFilePath : "");
}



//RECORDING FUNCTION
void CinematicsBuddy::RecordingFunction()
{
    const FrameInfo& ThisFrame = FrameInfo::Get();
	
    if(Exporter && Exporter->GetbIsRecording())
    {
        Exporter->AddData(ThisFrame);
    }

    if(Buffer && Buffer->GetbIsRecording())
    {
        Buffer->AddData(ThisFrame);
    }
}

//NORMAL RECORDING
void CinematicsBuddy::RecordStart()
{
    Exporter->StartRecording(GetSpecialFilePath(), *ExportFileName, *ExportCameraName);
}
void CinematicsBuddy::RecordStop()
{
    Exporter->StopRecording();
}

//BUFFER RECORDING
void CinematicsBuddy::BufferStart()
{
    Buffer->StartRecording(GetSpecialFilePath(), *ExportFileName, *ExportCameraName);
}
void CinematicsBuddy::BufferCapture()
{
    Buffer->CaptureBuffer(GetSpecialFilePath(), *ExportFileName, *ExportCameraName);
}
void CinematicsBuddy::BufferCancel()
{
	Buffer->StopRecording();
}


/* INPUT OVERRIDE */
void CinematicsBuddy::PlayerInputTick()
{
	if(!gameWrapper->IsInReplay()) return;
	
	if(*bUseCamVelocity)
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
