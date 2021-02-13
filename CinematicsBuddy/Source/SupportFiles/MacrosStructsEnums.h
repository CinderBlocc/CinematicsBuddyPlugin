#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"


//Allow usage of cvarManager and gameWrapper in any file that includes this file
//These pointers are declared and assigned at the top of CinematicsBuddy.cpp
extern std::shared_ptr<CVarManagerWrapper> GlobalCvarManager;
extern std::shared_ptr<GameWrapper>        GlobalGameWrapper;


// MACROS //
#define PLUGIN_VERSION "0.9.8"
#define EXTENSION_NAME ".txt"

//Cvars the user can set
#define CVAR_SET_SPECIAL_PATH    "CB_bSetFilePath"
#define CVAR_SPECIAL_PATH        "CB_FilePath"
#define CVAR_INCREMENT_FILES     "CB_bIncrementFileNames"
#define CVAR_FILE_NAME           "CB_FileName"
#define CVAR_CAMERA_NAME         "CB_CameraName"
#define CVAR_MAX_RECORD_LENGTH   "CB_MaxRecordingLength"
#define CVAR_MAX_BUFFER_LENGTH   "CB_MaxBufferLength"
#define CVAR_BUFFER_ENABLED      "CB_bBufferEnabled"
#define CVAR_IMPORT_FILE_NAME    "CB_ImportFileName"
#define CVAR_ENABLE_CAM_OVERRIDE "CB_bUseCamOverrides"
#define CVAR_CAM_MOVEMENT_SPEED  "CB_CamMovementSpeed"
#define CVAR_CAM_ROTATION_SPEED  "CB_CamRotationSpeed"

//Cvars only the plugin should set internally
#define CVAR_IS_RECORDING_ACTIVE "CB_bIsRecordingActive"
#define CVAR_IS_FILE_WRITING     "CB_bIsFileWriting"

//Notifiers
#define NOTIFIER_RECORD_START   "CBRecordStart"
#define NOTIFIER_RECORD_STOP    "CBRecordStop"
#define NOTIFIER_BUFFER_CAPTURE "CBBufferCapture"
#define NOTIFIER_IMPORT_FILE    "CBAnimationImport"
#define NOTIFIER_IMPORT_CLEAR   "CBAnimationClear"


// STRUCTS //


// ENUMS //
