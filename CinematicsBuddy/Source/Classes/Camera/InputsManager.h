#pragma once
#include <string>
#include <chrono>
#include <memory>

class PlayerControllerWrapper;
class CameraConfigManager;

enum class EInputSwapType
{
    S_NONE = 0,
    S_Forward,
    S_Right,
    S_Up,
    S_Pitch,
    S_Yaw
};

class InputsManager
{
public:
    InputsManager();

    //Get input values from game
    void PlayerInputTick(float Delta);

    //Give input values to other classes
    float GetForward()       { return Forward;       }
    float GetRight()         { return Right;         }
    float GetUp()            { return Up;            }
    float GetPitch()         { return Pitch;         }
    float GetYaw()           { return Yaw;           }
    float GetRoll()          { return Roll;          }
    float GetFOV()           { return FOV;           }
    float GetbRoll()         { return bRoll;         }
    float GetbFOV()          { return bFOV;          }
    bool  GetbUsingGamepad() { return bUsingGamepad; }

private:
    //Get and manipulate input values from game
    void GetInputs(PlayerControllerWrapper Controller);
    void NullifyInputs(PlayerControllerWrapper Controller);

    //Inputs read from game (and modified)
    float Forward = 0.f;
    float Right   = 0.f;
    float Up      = 0.f;
    float Pitch   = 0.f;
    float Yaw     = 0.f;
    float Roll    = 0.f;
    float FOV     = 0.f;
    bool  bUsingGamepad = false;

    //Variables for input swapping and manipulation
    std::shared_ptr<bool> bInvertPitch = std::make_shared<bool>(false);
    std::shared_ptr<std::string> RollBinding    = std::make_shared<std::string>("XboxTypeS_RightShoulder");
    std::shared_ptr<std::string> FOVBinding     = std::make_shared<std::string>("XboxTypeS_LeftShoulder");
    std::shared_ptr<std::string> RollSwapChoice = std::make_shared<std::string>("Yaw");
    std::shared_ptr<std::string> FOVSwapChoice  = std::make_shared<std::string>("Right");
    EInputSwapType RollSwap = EInputSwapType::S_NONE;
    EInputSwapType FOVSwap  = EInputSwapType::S_NONE;
    int  RollBindingIndex = 0;
    int  FOVBindingIndex  = 0;
    bool bRoll = false;
    bool bFOV  = false;

    //Functions for input swapping
    void SetBindingOptions();
    void SetInputSwapOptions();
    void CacheRollBinding();
    void CacheFOVBinding();
    void CacheRollSwap();
    void CacheFOVSwap();
    void DoSwap(EInputSwapType SwapType, float& ValueToSwap);
    EInputSwapType GetSwapType(const std::string& InTypeString);
};
