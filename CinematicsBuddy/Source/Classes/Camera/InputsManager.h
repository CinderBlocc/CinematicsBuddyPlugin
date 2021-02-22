#pragma once
#include <string>
#include <chrono>
#include <memory>

class PlayerControllerWrapper;
class CameraConfigManager;
class UIManager;

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
    InputsManager(std::shared_ptr<UIManager> TheUI);

    //Get input values from game
    void PlayerInputTick(float Delta);

    //Give input values to other classes
    float GetForward()       { return m_Forward;       }
    float GetRight()         { return m_Right;         }
    float GetUp()            { return m_Up;            }
    float GetPitch()         { return m_Pitch;         }
    float GetYaw()           { return m_Yaw;           }
    float GetRoll()          { return m_Roll;          }
    float GetFOV()           { return m_FOV;           }
    float GetbRoll()         { return m_bRoll;         }
    float GetbFOV()          { return m_bFOV;          }
    bool  GetbUsingGamepad() { return m_bUsingGamepad; }

private:
    std::shared_ptr<UIManager> UI;

    //Get and manipulate input values from game
    void GetInputs(PlayerControllerWrapper Controller);
    void NullifyInputs(PlayerControllerWrapper Controller);

    //Inputs read from game (and modified)
    float m_Forward = 0.f;
    float m_Right   = 0.f;
    float m_Up      = 0.f;
    float m_Pitch   = 0.f;
    float m_Yaw     = 0.f;
    float m_Roll    = 0.f;
    float m_FOV     = 0.f;
    bool m_bUsingGamepad = false;

    //Variables for input swapping and manipulation
    std::shared_ptr<bool> m_bInvertPitch = std::make_shared<bool>(false);
    std::shared_ptr<std::string> m_RollBinding    = std::make_shared<std::string>("XboxTypeS_RightShoulder");
    std::shared_ptr<std::string> m_FOVBinding     = std::make_shared<std::string>("XboxTypeS_LeftShoulder");
    std::shared_ptr<std::string> m_RollSwapChoice = std::make_shared<std::string>("Yaw");
    std::shared_ptr<std::string> m_FOVSwapChoice  = std::make_shared<std::string>("Right");
    EInputSwapType m_RollSwap = EInputSwapType::S_NONE;
    EInputSwapType m_FOVSwap  = EInputSwapType::S_NONE;
    int m_RollBindingIndex = 0;
    int m_FOVBindingIndex  = 0;
    bool m_bRoll = false;
    bool m_bFOV  = false;

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
