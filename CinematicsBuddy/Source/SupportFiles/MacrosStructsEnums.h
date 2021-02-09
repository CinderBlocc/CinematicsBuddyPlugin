#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <chrono>


extern std::shared_ptr<CVarManagerWrapper> GlobalCvarManager;
extern std::shared_ptr<GameWrapper>        GlobalGameWrapper;


// MACROS //
#define PLUGIN_VERSION "0.9.7"
#define EXTENSION_NAME ".txt"


// STRUCTS //
struct FileHeaderInfo
{
    std::string Print() const
    {
        return "BEGIN ANIMATION";
    }
};


// ENUMS //
