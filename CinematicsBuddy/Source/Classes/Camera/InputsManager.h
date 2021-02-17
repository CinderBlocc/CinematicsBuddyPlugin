#pragma once
#include <string>
#include <chrono>
#include <memory>

class PlayerControllerWrapper;

class InputsManager
{
public:
    //Get input values from game
    void PlayerInputTick(float Delta, bool bRoll);

    //Give input values to other classes
    float GetForward()       { return Forward;       }
    float GetRight()         { return Right;         }
    float GetUp()            { return Up;            }
    float GetPitch()         { return Pitch;         }
    float GetYaw()           { return Yaw;           }
    float GetRoll()          { return Roll;          }
    bool  GetbUsingGamepad() { return bUsingGamepad; }

    // TESTS - REMOVE WHEN DONE //
    void RunTest()  { TestStartTime = std::chrono::steady_clock::now(); bTestIsRunning = true;  }
    void HandleTest();
    std::chrono::steady_clock::time_point TestStartTime;
    bool bTestIsRunning = false;

private:
    void GetInputs(PlayerControllerWrapper Controller, bool bRoll);
    void NullifyInputs(PlayerControllerWrapper Controller);

    //Inputs read from game (and modified)
    float Forward = 0.f;
    float Right   = 0.f;
    float Up      = 0.f;
    float Pitch   = 0.f;
    float Yaw     = 0.f;
    float Roll    = 0.f;

    //State variables read from game
    bool bUsingGamepad = false;
};
