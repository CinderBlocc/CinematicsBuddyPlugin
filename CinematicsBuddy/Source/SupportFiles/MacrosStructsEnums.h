#pragma once


//Allow usage of cvarManager and gameWrapper in any file that includes this file
//These pointers are declared and assigned at the top of CinematicsBuddy.cpp
extern std::shared_ptr<class CVarManagerWrapper> GlobalCvarManager;
extern std::shared_ptr<class GameWrapper>        GlobalGameWrapper;


// MACROS //
#define PLUGIN_VERSION      "0.9.8"

//#TODO: Remove this in final release
#define IS_GAMMA_BUILD

//Generic defines
#define EXTENSION_RECORDING ".txt"
#define EXTENSION_CONFIG    ".cfg"
#define NO_SELECTION        "-- NONE --"

//Mutual recording cvars
#define CVAR_INCREMENT_FILES     "CB_bIncrementFileNames"
#define CVAR_SET_SPECIAL_PATH    "CB_bSetFilePath"
#define CVAR_SPECIAL_PATH        "CB_FilePath"
#define CVAR_FILE_NAME           "CB_FileName"
#define CVAR_CAMERA_NAME         "CB_CameraName"
#define CVAR_IS_FILE_WRITING     "CB_bIsFileWriting"

//Exporting cvars and notifiers
#define CVAR_IS_RECORDING_ACTIVE "CB_Recording_bIsActive"
#define CVAR_MAX_RECORD_LENGTH   "CB_Recording_MaxLength"
#define NOTIFIER_RECORD_START    "CBRecordingStart"
#define NOTIFIER_RECORD_STOP     "CBRecordingStop"

//Buffer cvars and notifiers
#define CVAR_BUFFER_ENABLED      "CB_Buffer_bEnabled"
#define CVAR_MAX_BUFFER_LENGTH   "CB_Buffer_MaxLength"
#define NOTIFIER_BUFFER_CAPTURE  "CBBufferCapture"
#define NOTIFIER_BUFFER_CLEAR    "CBBufferClear"

//Importing cvars and notifiers
#define CVAR_IMPORT_FILE_NAME    "CB_Import_FileName"
#define NOTIFIER_IMPORT_FILE     "CBImportAnimation"
#define NOTIFIER_IMPORT_CLEAR    "CBImportClear"

//Camera override cvars
#define CVAR_ENABLE_CAM_OVERRIDE "CB_Camera_bUseCamOverrides"
#define CVAR_CAM_LOCAL_MOVEMENT  "CB_Camera_bUseLocalMovement"
#define CVAR_CAM_LOCAL_ROTATION  "CB_Camera_bUseLocalRotation"
#define CVAR_CAM_HARD_FLOORS     "CB_Camera_bHardFloors"
#define CVAR_INVERT_PITCH        "CB_Camera_InvertControllerPitch"
#define CVAR_CAM_LOCAL_MOMENTUM  "CB_Camera_bLocalMomentum"
#define CVAR_CAM_FLOOR_HEIGHT    "CB_Camera_FloorHeight"
#define CVAR_CAM_MOVEMENT_SPEED  "CB_Camera_MovementSpeed"
#define CVAR_CAM_MOVEMENT_ACCEL  "CB_Camera_MovementAccel"
#define CVAR_ROT_SPEED_MOUSE     "CB_Camera_RotationSpeedMouse"
#define CVAR_ROT_SPEED_GAMEPAD   "CB_Camera_RotationSpeedGamepad"
#define CVAR_ROT_ACCEL_MOUSE     "CB_Camera_RotationAccelMouse"
#define CVAR_ROT_ACCEL_GAMEPAD   "CB_Camera_RotationAccelGamepad"
#define CVAR_FOV_ROTATION_SCALE  "CB_Camera_FOVRotationScale"
#define CVAR_FOV_MIN             "CB_Camera_FOVMin"
#define CVAR_FOV_MAX             "CB_Camera_FOVMax"
#define CVAR_FOV_SPEED           "CB_Camera_FOVSpeed"
#define CVAR_FOV_ACCELERATION    "CB_Camera_FOVAcceleration"
#define CVAR_FOV_LIMIT_EASE      "CB_Camera_FOVLimitEase"
#define CVAR_ROLL_BINDING        "CB_Camera_RollBinding"
#define CVAR_ROLL_SWAP           "CB_Camera_RollSwap"
#define CVAR_FOV_BINDING         "CB_Camera_FOVBinding"
#define CVAR_FOV_SWAP            "CB_Camera_FOVSwap"

//Camera config cvars and notifiers
#define CVAR_CONFIG_CURRENT      "CB_Config_Current"
#define CVAR_CONFIG_NEW_NAME     "CB_Config_NewName"
#define NOTIFIER_CONFIG_SAVE     "CBConfigSave"
#define NOTIFIER_CONFIG_UPDATE   "CBConfigUpdateList"

//Macros for simplifying cvar and notifier creation
#define MAKE_CVAR(...) GlobalCvarManager->registerCvar(##__VA_ARGS__)
#define MAKE_CVAR_BIND_STRING(cvar, cvarname, description, ...) GlobalCvarManager->registerCvar(cvarname, *cvar, description, ##__VA_ARGS__).bindTo(cvar)
#define MAKE_CVAR_BIND_TO_STRING(cvar, cvarname, description, ...) GlobalCvarManager->registerCvar(cvarname, std::to_string(*cvar), description, ##__VA_ARGS__).bindTo(cvar)
#define ON_CVAR_CHANGED(cvarname, funcname) GlobalCvarManager->getCvar(cvarname).addOnValueChanged(std::bind(&funcname, this))
#define MAKE_NOTIFIER(notifiername, funcname, description) GlobalCvarManager->registerNotifier(notifiername, [this](std::vector<std::string> params){funcname();}, description, PERMISSION_ALL);
