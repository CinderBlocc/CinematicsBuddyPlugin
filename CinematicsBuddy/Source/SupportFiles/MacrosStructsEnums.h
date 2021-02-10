#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"


//Allow usage of cvarManager and gameWrapper in any file that includes this file
//These pointers are declared and assigned at the top of CinematicsBuddy.cpp
extern std::shared_ptr<CVarManagerWrapper> GlobalCvarManager;
extern std::shared_ptr<GameWrapper>        GlobalGameWrapper;


// MACROS //
#define PLUGIN_VERSION "0.9.8"
#define EXTENSION_NAME ".txt"

#define CVAR_SET_SPECIAL_PATH   "CB_b_set_file_path"
#define CVAR_SPECIAL_PATH       "CB_file_path"
#define CVAR_FILE_NAME          "CB_file_name"
#define CVAR_CAMERA_NAME        "CB_camera_name"
#define CVAR_MAX_RECORD_LENGTH  "CB_record_length"
#define CVAR_MAX_BUFFER_LENGTH  "CB_buffer_length"
#define CVAR_IMPORT_FILE_NAME   "CB_import_file_name"
#define CVAR_SMOOTH_CAM_INPUTS  "CB_use_cam_velocity"
#define CVAR_CAM_MOVEMENT_SPEED "CB_cam_speed"
#define CVAR_CAM_ROTATION_SPEED "CB_cam_speed_rotation"
#define CVAR_SHOW_VERSION_INFO  "CB_show_version_info"
	
#define NOTIFIER_RECORD_START   "CBRecordStart"
#define NOTIFIER_RECORD_STOP    "CBRecordStop"
#define NOTIFIER_BUFFER_START   "CBBufferStart"
#define NOTIFIER_BUFFER_CAPTURE "CBBufferCapture"
#define NOTIFIER_BUFFER_STOP    "CBBufferCancel"
#define NOTIFIER_IMPORT_FILE    "CBAnimationImport"
#define NOTIFIER_IMPORT_CLEAR   "CBAnimationClear"
#define NOTIFIER_TEST_EXPORT    "CBTestExportFormat"


// STRUCTS //


// ENUMS //
