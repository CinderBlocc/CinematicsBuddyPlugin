#include "CinematicsBuddy.h"
#include "bakkesmod\wrappers\includes.h"
//#include "HelperFunctions.h"
#include <fstream>
#include <iomanip>
#include <sstream>
//#define _USE_MATH_DEFINES
//#include <math.h>

using namespace std::chrono;


/*
	PRIORITY

	- use SimpleJSON to format output

	- USE LINKED LIST OR SOMETHING OTHER THAN vector<> (or maybe not? Need to learn more about how vector works)
		- Every time you call push_back() its reallocating the entire set of memory which gets more expensive the longer you record
		- Something like a linked list will only add that one bit of memory and then point to it
			- The issue with linked list will be in a slowdown when writing the file, but that won't affect performance during recording
			- DO A MUCH MORE IN-DEPTH STUDY OF DATA STRUCTURES BEFORE LANDING ON THE FINAL STRUCTURE TO USE FOR THIS

	- File incrementation option

	- Add GetLoadoutBody back into the plugin once bakkes fixes the BM side of it

	- Speed up file output with this?
		- std::cout.sync_with_stdio(false): found in a comment on this video: https://www.youtube.com/watch?v=oEx5vGNFrLk
		- or just use '\n' instead of std::endl so that it doesnt flush buffer constantly
*/

/*
TO-DO:
	- LOCAL CAMERA MOVEMENT: As soon as the camera has rolled in flycam, it's gg trying to control it anymore
		- Add an option to override movement so that inputs will be relative to camera's rotation
		- i.e. holding trigger will move camera along local up axis instead of world up axis
		- i.e. pitch and yaw will be local instead of global after camera has rolled

	- Camera animation importing
		- INPORT INTERPOLATION
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

	- Export car mesh animation instead of car rigid body animation
		- This will make the car align better in external software
		- Do the same for the ball mesh since it doesn't seem to line up very well at times either
*/

//0.9.7 - export rewrite
//0.9.8 - import rewrite
//0.9.9 - input smoothing
//1.0.0 - car mesh export? Definitely wrap things up like error rendering before 1.0.0
//		- REMOVE VERSION DEPENDENCIES IN 1.0.0!!! Completely lock in the formatting of the text file before version 1 so all future updates dont rely on a broken version
//				- Still include version numbers in the file though in case those need to be referenced in troubleshooting
std::string cppVersion = "0.9.7";
BAKKESMOD_PLUGIN(CinematicsBuddy, "Cinematics Buddy Plugin", cppVersion.c_str(), PLUGINTYPE_REPLAY)

std::string defaultExportPath = "./bakkesmod/data/CinematicsBuddy/AnimationExports/";
std::string defaultImportPath = "./bakkesmod/data/CinematicsBuddy/AnimationImports/";
void CinematicsBuddy::onLoad()
{
	cvarFileName         = std::make_shared<std::string>("");
	cvarCameraName       = std::make_shared<std::string>("");
	cvarBufferSize       = std::make_shared<float>(0.f);
	cvarImportFileName   = std::make_shared<std::string>("");
	useCamVelocity       = std::make_shared<bool>(false);
	cvarCamSpeed         = std::make_shared<float>(1.0f);
	cvarCamRotationSpeed = std::make_shared<float>(1.0f);
	cvarShowVersionInfo  = std::make_shared<bool>(false);

	cvarManager->registerCvar("cb_file_name", "", "Set the output file name", true, false, 0, false, 0).bindTo(cvarFileName);
	cvarManager->registerCvar("cb_camera_name", "", "Set the camera name", true, false, 0, false, 0).bindTo(cvarCameraName);
	cvarManager->registerCvar("cb_buffer_size", "160", "Number of seconds to buffer", true, true, 0, true, 6000).bindTo(cvarBufferSize);
	cvarManager->registerCvar("cb_import_file_name", "", "Set the output file name", true, false, 0, false, 0).bindTo(cvarImportFileName);
	cvarManager->registerCvar("cb_use_cam_velocity", "0", "Smooth camera movements", true, false, 0, false, 0).bindTo(useCamVelocity);
	cvarManager->registerCvar("cb_cam_speed", "1", "Camera speed multiplier", true, true, 0, true, 1).bindTo(cvarCamSpeed);
	cvarManager->registerCvar("cb_cam_speed_rotation", "1", "Camera rotation speed multiplier", true, true, 0, true, 1).bindTo(cvarCamRotationSpeed);
	cvarManager->registerCvar("cb_show_version_info", "0", "Display version information on screen", true, false, 0, false, 0).bindTo(cvarShowVersionInfo);
	
	cvarManager->registerNotifier("cbRecordStart",     [this](std::vector<std::string> params){RecordStart();}, "Starts capturing animation data.", PERMISSION_ALL);
	cvarManager->registerNotifier("cbRecordStop",      [this](std::vector<std::string> params){RecordStop();}, "Stops capturing animation data", PERMISSION_ALL);
	cvarManager->registerNotifier("cbBufferStart",     [this](std::vector<std::string> params){BufferStart();}, "Starts the perpetual animation capture buffer", PERMISSION_ALL);
	cvarManager->registerNotifier("cbBufferCapture",   [this](std::vector<std::string> params){BufferCapture();}, "Captures the data in the buffer", PERMISSION_ALL);
	cvarManager->registerNotifier("cbBufferCancel",    [this](std::vector<std::string> params){BufferCancel();}, "Cancels the perpetual animation buffer", PERMISSION_ALL);
	cvarManager->registerNotifier("cbAnimationImport", [this](std::vector<std::string> params){CamPathImport();}, "Imports a camera animation from a file", PERMISSION_ALL);
	cvarManager->registerNotifier("cbAnimationClear",  [this](std::vector<std::string> params){CamPathClear();}, "Clears the imported camera animation", PERMISSION_ALL);

	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", std::bind(&CinematicsBuddy::RecordingFunction, this));
	gameWrapper->HookEvent("Function TAGame.PlayerInput_TA.PlayerInput", std::bind(&CinematicsBuddy::PlayerInputTick, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.PlayerInput_TA.PlayerInput", bind(&CinematicsBuddy::PlayerInputTick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	gameWrapper->RegisterDrawable(bind(&CinematicsBuddy::Render, this, std::placeholders::_1));
}
void CinematicsBuddy::onUnload(){}


/* RECORDING */
bool recording = false;
bool bufferIsActive = false;

//RECORDING FUNCTION
void CinematicsBuddy::RecordingFunction()
{
	if(!recording && !bufferIsActive) return;
	ServerWrapper gameState = GetCurrentGameState();
	CameraWrapper camera = gameWrapper->GetCamera();
	if(camera.IsNull() || gameState.IsNull()) return;
		
	/* GET THE DATA */
	//Time
	TimeInfo timeInfo;
	timeInfo.timeCaptured = steady_clock::now();
	if(gameWrapper->IsInReplay())
		timeInfo.replayFrame = gameWrapper->GetGameEventAsReplay().GetCurrentReplayFrame();
	else
		timeInfo.replayFrame = -1;
	
	//Camera
	CameraInfo cameraInfo;
	cameraInfo.location = camera.GetLocation();
	cameraInfo.orientation = RotatorToQuat(camera.GetRotation());
	cameraInfo.FOV = camera.GetFOV();
	
	//Ball
	BallInfo ballInfo;
	if(!gameState.GetBall().IsNull())
	{
		BallWrapper ball = gameState.GetBall();
		ballInfo.location = ball.GetLocation();
		ballInfo.orientation = RotatorToQuat(ball.GetRotation());
	}
	
	//Cars
	ArrayWrapper<CarWrapper> cars = gameState.GetCars();
	std::vector<CarInfo> allCarInfo;
	std::vector<CarsSeen> carsSeenThisFrame;
	for(int i=0; i<cars.Count(); i++)
	{
		CarWrapper car = cars.Get(i);
		if(car.IsNull()) continue;
		if(car.GetPRI().IsNull()) continue;

		CarsSeen thisCarSeen;
		thisCarSeen.timeSeen = steady_clock::now();
		thisCarSeen.ID = car.GetPRI().GetUniqueId();
		thisCarSeen.body = car.GetLoadoutBody();

		CarInfo thisCarInfo;
		thisCarInfo.isBoosting = car.IsBoostCheap();
		thisCarInfo.location = car.GetLocation();
		thisCarInfo.orientation = RotatorToQuat(car.GetRotation());
		thisCarInfo.ID = car.GetPRI().GetUniqueId();
		
		//Wheels
		ArrayWrapper<WheelWrapper> wheels = car.GetVehicleSim().GetWheels();
		for(int j=0;j<4;j++)
		{
			WheelWrapper wheel = wheels.Get(j);
			thisCarInfo.wheels[j].steer = wheel.GetSteer2();
			thisCarInfo.wheels[j].suspensionDistance = wheel.GetSuspensionDistance();
			thisCarInfo.wheels[j].spinSpeed = wheel.GetSpinSpeed();

			if(j==0) thisCarSeen.wheels01Radius = wheel.GetWheelRadius();
			if(j==2) thisCarSeen.wheels23Radius = wheel.GetWheelRadius();

			//MESH SUSPENSION?
			/*
			ARATOR:@CinderBlock actually I think it's Car->CarMesh->WheelControls.Data[i]->TranslationControl->BoneTranslation
			so this would need to be wrapped in the SDK
			but
			for now you can use these evil hacks
			```
			//	unsigned char* CarMeshPtr = (unsigned char*)car.memory_address + 0x608;
			//	uintptr_t CarMeshMemAddr = *((uintptr_t*)CarMeshPtr);    
			//
			//	unsigned char* WheelControlsPtr = (unsigned char*)CarMeshMemAddr + 0x73C;
			//	uintptr_t WheelControlsMemAddr = *((uintptr_t*)WheelControlsPtr);
			//
			//	Vector BoneTranslation[4];
			//	for (int i = 0; i < 4; i++)
			//	{
			//		uintptr_t MemAddr = WheelControlsMemAddr + i * 0x10;
			//		unsigned char* TranslationControlPtr = (unsigned char*)MemAddr + 0x4;
			//
			//		uintptr_t TranslationControlMemAddr = *((uintptr_t*)TranslationControlPtr);
			//		BoneTranslation[i] = *((Vector*)((unsigned char*)TranslationControlMemAddr + 0xC0));
			//	}
			```
			assuming you have your car object in a variable named car, it gives you an array of vectors (one for each wheel)
			im pretty sure the Z-value of each vector gives the current visual suspension distance of that wheel
			*/
		}
		
		carsSeenThisFrame.push_back(thisCarSeen);
		allCarInfo.push_back(thisCarInfo);
	}

	/* STORE THE DATA */
	FrameInfo frameInfo;
	frameInfo.timeCaptured = steady_clock::now();
	if(gameWrapper->IsInReplay())
	{
		ReplayWrapper replay = gameWrapper->GetGameEventAsReplay().GetReplay();
		if(replay.memory_address != NULL)
			frameInfo.replayID = replay.GetId().ToString();
	}
	frameInfo.timeInfo = timeInfo;
	frameInfo.cameraInfo = cameraInfo;
	frameInfo.ballInfo = ballInfo;
	frameInfo.carInfo = allCarInfo;
	frameInfo.carsSeenInfo = carsSeenThisFrame;

	//Formatting for recording
	if(recording)
	{		
		recordingFrames.push_back(frameInfo);

		duration<double> currentRecordingLength = duration_cast<duration<double>>(steady_clock::now() - recordingFrames[0].timeCaptured);
		if(currentRecordingLength.count() >= 600)//Max 10 minutes of recording (600 seconds)
			RecordStop();
	}

	//Formatting for buffer
	if(bufferIsActive)
	{
		bufferFrames.push_back(frameInfo);

		duration<double> currentBufferSize = duration_cast<duration<double>>(steady_clock::now() - bufferFrames[0].timeCaptured);
		if(currentBufferSize.count() >= *cvarBufferSize)
			bufferFrames.erase(bufferFrames.begin());
	}
}

//NORMAL RECORDING
void CinematicsBuddy::RecordStart()
{
	if(recording) return;

	bool canStartRecording = true;
	if (*cvarFileName == "")
	{
		warnings.push_back("INVALID FILE NAME");
		canStartRecording = false;
	}
	if (*cvarCameraName == "")
	{
		warnings.push_back("INVALID CAMERA NAME");
		canStartRecording = false;
	}

	if(canStartRecording)
		recording = true;
}
void CinematicsBuddy::RecordStop()
{
	std::string filename = defaultExportPath + *cvarFileName + ".txt";

	duration<double> recordingDuration = duration_cast<duration<double>>(steady_clock::now() - recordingFrames[0].timeCaptured);
	warnings.push_back("Recording stopped: " + std::to_string(recordingDuration.count()) + " seconds, " + std::to_string(recordingFrames.size()) + " frames");

	WriteToFile(recordingFrames, filename);
}

//BUFFER RECORDING
void CinematicsBuddy::BufferStart()
{
	if(!bufferIsActive) bufferFrames.clear();
	bufferIsActive = true;
}
void CinematicsBuddy::BufferCapture()
{
	std::string filename = defaultExportPath + "cbBuffer_" + GetBufferFilenameTime() + ".txt";

	duration<double> bufferDuration = duration_cast<duration<double>>(steady_clock::now() - bufferFrames[0].timeCaptured);
	warnings.push_back("Buffer captured: " + std::to_string(bufferDuration.count()) + " seconds, " + std::to_string(bufferFrames.size()) + " frames");

	WriteToFile(bufferFrames, filename);
}
void CinematicsBuddy::BufferCancel()
{
	bufferIsActive = false;
	bufferFrames.clear();
}


/* INPUT OVERRIDE */
//void CinematicsBuddy::PlayerInputTick(ActorWrapper camInput, void * params, string funcName)
void CinematicsBuddy::PlayerInputTick()
{
	if(!gameWrapper->IsInReplay()) return;
	
	if(*useCamVelocity)
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


/* FILE WRITING */
const std::string tab = " ", n = "\n";
void CinematicsBuddy::WriteToFile(std::vector<FrameInfo> frames, std::string filename)
{
	std::ofstream outputFile(defaultExportPath + *cvarFileName + ".txt");

	//Scan frames list for replay and car info
	bool wasEntirelyInReplay = true;
	std::string firstReplayID = frames[0].replayID;
	std::vector<CarsSeen> carsSeenInThisRecording;
	for(int i=0; i<frames.size(); i++)
	{
		//Check if the recording was in the same replay the whole time
		if(wasEntirelyInReplay)
		{
			if(frames[i].replayID == "NULL" || frames[i].replayID != firstReplayID)
				wasEntirelyInReplay = false;
		}

		//Loop through all cars seen in this frame, compare to list of unique cars
		for(int j=0; j<frames[i].carsSeenInfo.size(); j++)//cars seen this frame
		{
			CarsSeen thisCar = frames[i].carsSeenInfo[j];
			bool isThisCarInList = false;
			for(int k=0; k<carsSeenInThisRecording.size(); k++)//list of unique cars seen
			{
				if(thisCar.ID.ID == carsSeenInThisRecording[k].ID.ID)
				{
					isThisCarInList = true;
					break;
				}
			}

			if(!isThisCarInList)
				carsSeenInThisRecording.push_back(thisCar);
		}
	}

	/* WRITE HEADER INFO */
	duration<double> recordingDuration = duration_cast<duration<double>>(frames[frames.size()-1].timeCaptured - frames[0].timeCaptured);
	float durationLength = recordingDuration.count();//Convert to seconds

	outputFile << "Version: " << cppVersion << std::endl;
	outputFile << "Camera: " << *cvarCameraName << std::endl;
	outputFile << "Duration: " << PrintFloat(durationLength, 3) << " seconds" << std::endl;
	outputFile << std::endl;

	//If the recording was all within the same replay, write the replay metadata to the header
	if(wasEntirelyInReplay)
	{
		ReplayWrapper replay = gameWrapper->GetGameEventAsReplay().GetReplay();
		if(replay.memory_address != NULL)
		{
			outputFile << "BEGIN REPLAY METADATA" << std::endl;
			outputFile << "Name: " << (replay.GetReplayName().IsNull() ? "NULL" : replay.GetReplayName().ToString()) << std::endl;
			outputFile << "ID: " << replay.GetId().ToString() << std::endl;
			outputFile << "Date: " << replay.GetDate().ToString() << std::endl;
			outputFile << "FPS: " << std::to_string(replay.GetRecordFPS()) << std::endl;
			outputFile << "Frames: " << std::to_string(replay.GetNumFrames()) << std::endl;
			outputFile << "END REPLAY METADATA" << std::endl;
			outputFile << std::endl;
		}
	}

	//Write the list of cars to the header
	outputFile << "BEGIN CAR LIST" << std::endl;
	for(int i=0; i<carsSeenInThisRecording.size(); i++)
	{
		outputFile << std::to_string(i) << "| ID[" << std::to_string(carsSeenInThisRecording[i].ID.ID) << "]"
		<< ", Body[" << std::to_string(carsSeenInThisRecording[i].body) << "]"
		<< ", Wheel radii[01:" << PrintFloat(carsSeenInThisRecording[i].wheels01Radius, 1) << "|23:" << PrintFloat(carsSeenInThisRecording[i].wheels23Radius, 1) << "]" << std::endl;
	}
	outputFile << "END CAR LIST" << std::endl;
	outputFile << std::endl;
	

	/* WRITE WHOLE ANIMATION */
	outputFile << "BEGIN ANIMATION" << std::endl;
	for(int i=0; i<frames.size(); i++)
		outputFile << FormatFrameData(i, frames[0], frames[i], carsSeenInThisRecording);
	outputFile << "END ANIMATION";
	
	outputFile.close();
}
std::string CinematicsBuddy::FormatFrameData(int index, FrameInfo firstFrame, FrameInfo currentFrame, std::vector<CarsSeen> carsList)
{
	std::string output;
	output += std::to_string(index) + "{" + n;
	
	//Time
	TimeInfo time = currentFrame.timeInfo;
	duration<double> delta = duration_cast<duration<double>>(currentFrame.timeCaptured - firstFrame.timeCaptured);
	output += tab+ "T[" + std::to_string(delta.count()) + "|" + std::to_string(time.replayFrame) + "]" + n;

	//Camera
	CameraInfo camera = currentFrame.cameraInfo;
	output += tab+ "C[" + PrintVector(camera.location, 2) + "|" + PrintQuat(camera.orientation, 6) + "|" + PrintFloat(camera.FOV, 3) + "]" + n;
	
	//Ball
	BallInfo ball = currentFrame.ballInfo;
	output += tab+ "B[" + PrintVector(ball.location, 2) + "|" + PrintQuat(ball.orientation, 6) + "]" + n;
	
	//Cars
	std::vector<CarInfo> cars = currentFrame.carInfo;
	output += tab+ "CS" + "(" + n;
	for(int i=0; i<cars.size(); i++)
	{
		int carIndex = -1;
		for(int j=0; j<carsList.size(); j++)
		{
			if(cars[i].ID.ID == carsList[j].ID.ID)
			{
				carIndex = j;
				break;
			}
		}
		output += tab+tab+ std::to_string(carIndex) + "(" + n;
		output += tab+tab+tab+ "BLR[" + std::to_string(cars[i].isBoosting) + "|" + PrintVector(cars[i].location, 2) + "|" + PrintQuat(cars[i].orientation, 6) + "]" + n;
		output += tab+tab+tab+ "WHL(" + n;
		for(int j=0; j<4; j++)
		{
			output += tab+tab+tab+tab+ std::to_string(j) + "[" + 
			PrintFloat(cars[i].wheels[j].steer, 3) + "|" +
			PrintFloat(cars[i].wheels[j].suspensionDistance, 3) + "|" +
			PrintFloat(cars[i].wheels[j].spinSpeed, 3) + "]" + n;
		}
		output += tab+tab+tab+ ")" + n;
		output += tab+tab+ ")" + n;
	}
	output += tab+ ")" + n;
	output += "}" + n;

	return output;
}
std::string CinematicsBuddy::GetBufferFilenameTime()
{
	time_t rawtime;
	struct tm *timestamp;
	char buffer [80];
	time(&rawtime);
	timestamp = localtime(&rawtime);
	strftime(buffer,80,"%Y-%m-%d_%H-%M-%S",timestamp);

	return std::string(buffer);
}


/* ANIMATION IMPORT */
bool hasDataVector = false;
bool stopApplyingAnimation = false;
std::vector<std::vector<float>> importDataVector;
void CinematicsBuddy::CamPathImport()
{
	//Ask user for confirmation if path metadata ID and current replay ID dont match
	if(!gameWrapper->IsInReplay()) return;
	ReplayWrapper replay = gameWrapper->GetGameEventAsReplay().GetReplay();
	if(replay.memory_address == NULL) return;
	std::ifstream inFile(defaultImportPath + "AnimationImports/" + *cvarImportFileName + ".txt");
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
			if(value != "")
				tempDataVector.push_back(atof(value.c_str()));
		}
		importDataVector.push_back(tempDataVector);
	}
	
	hasDataVector = true;
	stopApplyingAnimation = false;
	CamPathApply();
}
void CinematicsBuddy::CamPathApply()
{
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
}
void CinematicsBuddy::CamPathClear()
{
	hasDataVector = false;
	stopApplyingAnimation = true;
	importDataVector.clear();
}


/* UTILITY FUNCTIONS */
//Quat CinematicsBuddy::RotatorToQuat(Rotator convertToQuat) //pulled from here with some modifications: https://github.com/ncsoft/Unreal.js/issues/132)
//{
//	float uRotToDeg = 182.044449;
//	float DegToRadDiv2 = (M_PI/180)/2;
//	float sinPitch = sin((convertToQuat.Pitch/uRotToDeg)*DegToRadDiv2);
//	float cosPitch = cos((convertToQuat.Pitch/uRotToDeg)*DegToRadDiv2);
//	float sinYaw = sin((convertToQuat.Yaw/uRotToDeg)*DegToRadDiv2);
//	float cosYaw = cos((convertToQuat.Yaw/uRotToDeg)*DegToRadDiv2);
//	float sinRoll = sin((convertToQuat.Roll/uRotToDeg)*DegToRadDiv2);
//	float cosRoll = cos((convertToQuat.Roll/uRotToDeg)*DegToRadDiv2);
//	
//	Quat convertedQuat;
//	convertedQuat.X = (cosRoll*sinPitch*sinYaw) - (sinRoll*cosPitch*cosYaw);
//	convertedQuat.Y = ((-cosRoll)*sinPitch*cosYaw) - (sinRoll*cosPitch*sinYaw);
//	convertedQuat.Z = (cosRoll*cosPitch*sinYaw) - (sinRoll*sinPitch*cosYaw);
//	convertedQuat.W = (cosRoll*cosPitch*cosYaw) + (sinRoll*sinPitch*sinYaw);
//
//	return convertedQuat;
//}
ServerWrapper CinematicsBuddy::GetCurrentGameState()
{
	if(gameWrapper->IsInReplay())
		return gameWrapper->GetGameEventAsReplay().memory_address;
	else if(gameWrapper->IsInOnlineGame())
		return gameWrapper->GetOnlineGame();
	else
		return gameWrapper->GetGameEventAsServer();
}
void CinematicsBuddy::Render(CanvasWrapper canvas)
{
	if(recording) return;
	if(gameWrapper->IsInOnlineGame()) return;

	std::vector<std::string> drawStrings;

	if(*cvarShowVersionInfo)
	{
		drawStrings.push_back("CinematicsBuddy version: " + cppVersion);
		drawStrings.push_back("");
	}

	for(int i=0; i<warnings.size(); i++)
		drawStrings.push_back(warnings[i]);




	canvas.SetColor(0,255,0,255);
	Vector2 base = {50,50};
	for(int i=0; i<drawStrings.size(); i++)
	{
		canvas.SetPosition(base);
		canvas.DrawString(drawStrings[i]);
		base.Y += 20;
	}
}

std::string CinematicsBuddy::PrintFloat(const float InFloat, const int InDecimals)
{
    std::ostringstream Output;
	Output << std::fixed << std::setprecision(InDecimals) << InFloat;
	return Output.str();
}

std::string CinematicsBuddy::PrintVector(const Vector& InVector, const int InDecimals)
{
    std::ostringstream Output;
	Output << std::fixed << std::setprecision(InDecimals) << InVector.X << ", " << InVector.Y << ", " << InVector.Z;
	return Output.str();
}

std::string CinematicsBuddy::PrintQuat(const Quat& InQuat, const int InDecimals)
{
    std::ostringstream Output;
	Output << std::fixed << std::setprecision(InDecimals) << InQuat.W << ", " << InQuat.X << ", " << InQuat.Y << ", " << InQuat.Z;
	return Output.str();
}
