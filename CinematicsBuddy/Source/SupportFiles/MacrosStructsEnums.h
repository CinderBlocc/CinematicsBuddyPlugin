#pragma once


//Allow usage of cvarManager and gameWrapper in any file that includes this file
//These pointers are declared and assigned at the top of CinematicsBuddy.cpp
extern std::shared_ptr<class CVarManagerWrapper> GlobalCvarManager;
extern std::shared_ptr<class GameWrapper>        GlobalGameWrapper;


// MACROS //
#define PLUGIN_VERSION "0.9.8"
#define EXTENSION_NAME ".txt"

//Mutual recording cvars
#define CVAR_INCREMENT_FILES     "CB_bIncrementFileNames"
#define CVAR_SET_SPECIAL_PATH    "CB_bSetFilePath"
#define CVAR_SPECIAL_PATH        "CB_FilePath"
#define CVAR_FILE_NAME           "CB_FileName"
#define CVAR_CAMERA_NAME         "CB_CameraName"
#define CVAR_IS_FILE_WRITING     "CB_bIsFileWriting"

//Exporting cvars
#define CVAR_IS_RECORDING_ACTIVE "CB_Recording_bIsActive"
#define CVAR_MAX_RECORD_LENGTH   "CB_Recording_MaxLength"

//Buffer cvars
#define CVAR_BUFFER_ENABLED      "CB_Buffer_bEnabled"
#define CVAR_MAX_BUFFER_LENGTH   "CB_Buffer_MaxLength"

//Importin cvars
#define CVAR_IMPORT_FILE_NAME    "CB_Import_FileName"

//Camera override cvars
#define CVAR_ENABLE_CAM_OVERRIDE "CB_Camera_bUseCamOverrides"
#define CVAR_CAM_LOCAL_MATRIX    "CB_Camera_bUseLocalMatrix"
#define CVAR_CAM_MOVEMENT_SPEED  "CB_Camera_MovementSpeed"
#define CVAR_CAM_MOVEMENT_ACCEL  "CB_Camera_MovementAccel"
#define CVAR_ROT_SPEED           "CB_Camera_RotationSpeed"
#define CVAR_ROT_ACCEL_MOUSE     "CB_Camera_RotationAccelMouse"
#define CVAR_ROT_ACCEL_GAMEPAD   "CB_Camera_RotationAccelGamepad"
#define CVAR_MOUSE_SENSITIVITY   "CB_Camera_MouseSensitivity"
#define CVAR_GAMEPAD_SENSITIVITY "CB_Camera_GamepadSensitivity"
#define CVAR_FOV_ROTATION_SCALE  "CB_Camera_FOVRotationScale" //#TODO: The more zoomed in, the less sensitive the rotation
#define CVAR_ROLL_BINDING        "CB_Camera_RollBinding"

//Notifiers
#define NOTIFIER_RECORD_START   "CBRecordStart"
#define NOTIFIER_RECORD_STOP    "CBRecordStop"
#define NOTIFIER_BUFFER_CAPTURE "CBBufferCapture"
#define NOTIFIER_BUFFER_CLEAR   "CBBufferClear"
#define NOTIFIER_IMPORT_FILE    "CBAnimationImport"
#define NOTIFIER_IMPORT_CLEAR   "CBAnimationClear"

//Macros for simplifying cvar and notifier creation
#define MAKE_CVAR(...) GlobalCvarManager->registerCvar(##__VA_ARGS__)
#define MAKE_CVAR_BIND_STRING(cvar, cvarname, description, ...) GlobalCvarManager->registerCvar(cvarname, *cvar, description, ##__VA_ARGS__).bindTo(cvar)
#define MAKE_CVAR_BIND_TO_STRING(cvar, cvarname, description, ...) GlobalCvarManager->registerCvar(cvarname, std::to_string(*cvar), description, ##__VA_ARGS__).bindTo(cvar)
#define ON_CVAR_CHANGED(cvarname, classname, funcname) GlobalCvarManager->getCvar(cvarname).addOnValueChanged(std::bind(&classname::funcname, this))
#define MAKE_NOTIFIER(notifiername, funcname, description) GlobalCvarManager->registerNotifier(notifiername, [this](std::vector<std::string> params){funcname();}, description, PERMISSION_ALL);
