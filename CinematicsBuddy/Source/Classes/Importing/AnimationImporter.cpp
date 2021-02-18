#include "AnimationImporter.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "UI/UIManager.h"

AnimationImporter::AnimationImporter(std::shared_ptr<UIManager> TheUI)
{
    UI = TheUI;

    //Register cvars
    MAKE_CVAR_BIND_STRING(ImportFileName, CVAR_IMPORT_FILE_NAME, "Set the import file name", true);

    //Register notifiers
    MAKE_NOTIFIER(NOTIFIER_IMPORT_FILE,  ImportCameraAnimation, "Imports a camera animation from a file");
	MAKE_NOTIFIER(NOTIFIER_IMPORT_CLEAR, ClearCameraAnimation,  "Clears the imported camera animation");
}

void AnimationImporter::ImportCameraAnimation()
{
    //Import the file specified by *ImportFileName

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

void AnimationImporter::ApplyCameraAnimation()
{
    //Called per tick by plugin. Only apply if in valid time range and if bPlayAnimation is true

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

void AnimationImporter::ClearCameraAnimation()
{
    //Clear the cached data and set the bPlayAnimation flag to false

    /*
	hasDataVector = false;
	stopApplyingAnimation = true;
	importDataVector.clear();
    */
}
