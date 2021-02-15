#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"


//Allow usage of cvarManager and gameWrapper in any file that includes this file
//These pointers are declared and assigned at the top of CinematicsBuddy.cpp
extern std::shared_ptr<CVarManagerWrapper> GlobalCvarManager;
extern std::shared_ptr<GameWrapper>        GlobalGameWrapper;


// MACROS //
#define PLUGIN_VERSION "0.9.8"
#define EXTENSION_NAME ".txt"

//Recording cvars
#define CVAR_SET_SPECIAL_PATH    "CB_bSetFilePath"
#define CVAR_SPECIAL_PATH        "CB_FilePath"
#define CVAR_INCREMENT_FILES     "CB_bIncrementFileNames"
#define CVAR_FILE_NAME           "CB_FileName"
#define CVAR_CAMERA_NAME         "CB_CameraName"
#define CVAR_MAX_RECORD_LENGTH   "CB_MaxRecordingLength"
#define CVAR_MAX_BUFFER_LENGTH   "CB_MaxBufferLength"
#define CVAR_BUFFER_ENABLED      "CB_bBufferEnabled"
#define CVAR_IMPORT_FILE_NAME    "CB_ImportFileName"

//Camera override cvars
#define CVAR_ENABLE_CAM_OVERRIDE "CB_bUseCamOverrides"
#define CVAR_CAM_LOCAL_MATRIX    "CB_bUseLocalMatrix"
#define CVAR_CAM_MOVEMENT_SPEED  "CB_CamMovementSpeed"
#define CVAR_CAM_MOVEMENT_ACCEL  "CB_CamMovementAccel"
#define CVAR_CAM_ROTATION_ACCEL  "CB_CamRotationAccel"
#define CVAR_MOUSE_SENSITIVITY   "CB_MouseSensitivity"
#define CVAR_GAMEPAD_SENSITIVITY "CB_GamepadSensitivity"
#define CVAR_FOV_ROTATION_SCALE  "CB_FOVRotationScale" //#TODO: The more zoomed in, the less sensitive the rotation
#define CVAR_ROLL_BINDING        "CB_RollBinding" //#TODO: This should be a dropdown menu of controller buttons

//Cvars only the plugin should set internally
#define CVAR_IS_RECORDING_ACTIVE "CB_bIsRecordingActive"
#define CVAR_IS_FILE_WRITING     "CB_bIsFileWriting"

//Notifiers
#define NOTIFIER_RECORD_START   "CBRecordStart"
#define NOTIFIER_RECORD_STOP    "CBRecordStop"
#define NOTIFIER_BUFFER_CAPTURE "CBBufferCapture"
#define NOTIFIER_BUFFER_CLEAR   "CBBufferClear"
#define NOTIFIER_IMPORT_FILE    "CBAnimationImport"
#define NOTIFIER_IMPORT_CLEAR   "CBAnimationClear"


// STRUCTS //


// ENUMS //
enum class ERecordingSettingChanged
{
    R_bIncrement = 0,
    R_bBufferEnabled,
    R_MaxBufferLength,
    R_MaxRecordingLength
};

enum class ECamOverrideChanged
{
    C_bUseOverrides = 0,
    C_bUseLocalMatrix,
    C_MovementSpeed,
    C_MovementAccel,
    C_RotationAccel,
    C_MouseSensitivity,
    C_GamepadSensitivity,
    C_FOVRotationScale,
    C_RollBinding
};
