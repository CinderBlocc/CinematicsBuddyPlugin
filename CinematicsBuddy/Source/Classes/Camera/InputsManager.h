#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <memory>

class PlayerControllerWrapper;
class CanvasWrapper;

class InputsManager
{
public:
    InputsManager();

    void PlayerInputTick(float Delta, bool bRoll);

    // TESTS - REMOVE WHEN DONE //
    void DebugRender(CanvasWrapper Canvas, std::vector<std::string>& RenderStrings);
    void RunTest()  { TestStartTime = std::chrono::steady_clock::now(); bTestIsRunning = true;  }
    std::chrono::steady_clock::time_point TestStartTime;
    bool bTestIsRunning = false;

    //Get input values
    float GetForward() { return Forward; }
    float GetRight()   { return Right;   }
    float GetUp()      { return Up;      }
    float GetPitch()   { return Pitch;   }
    float GetYaw()     { return Yaw;     }
    float GetRoll()    { return Roll;    }

private:
    void GetInputs(PlayerControllerWrapper Controller, bool bRoll);
    void NullifyInputs(PlayerControllerWrapper Controller);

    //Inputs read from game (and modified)
    float Forward; // Default range: Gamepad(-1 to 1), Keyboard(-1 to 1)
    float Right;   // Default range: Gamepad(-1 to 1), Keyboard(-1 to 1)
    float Up;      // Default range: Gamepad(-1 to 1), Keyboard(-1 to 1)
    float Pitch;   // Default range: Gamepad(-1 to 1), Keyboard(-??? to ??? -- Depends on range of mouse movement)
    float Yaw;     // Default range: Gamepad(-1 to 1), Keyboard(-??? to ??? -- Depends on range of mouse movement)
    float Roll;    // Default range: Gamepad(-1 to 1), Keyboard(-??? to ??? -- Depends on range of mouse movement)

    //State variables read from game
    bool bUsingGamepad;
};
