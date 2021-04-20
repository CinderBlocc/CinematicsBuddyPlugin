#pragma once
#include <filesystem>

class BetaCleanup
{
public:
    static void RemoveBetaFiles();

    static void RemoveFile(std::filesystem::path TheFile);
    static void RemoveFolder(std::filesystem::path TheFolder);
};
