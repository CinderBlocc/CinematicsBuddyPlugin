#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <memory>

class CanvasWrapper;
class InputsManager;

class CameraManager
{
public:
    CameraManager();

    void PlayerInputTick(float Delta, bool InbRoll);
    
    //Set state values
    void SetbUseOverrides(bool bNewValue)      { bUseOverrides      = bNewValue; }
    void SetMovementSpeed(float NewValue)      { MovementSpeed      = NewValue;  }
    void SetMovementAccel(float NewValue)      { MovementAccel      = NewValue;  }
    void SetRotationAccel(float NewValue)      { RotationAccel      = NewValue;  }
    void SetMouseSensitivity(float NewValue)   { MouseSensitivity   = NewValue;  }
    void SetGamepadSensitivity(float NewValue) { GamepadSensitivity = NewValue;  }
    void SetFOVRotationScale(float NewValue)   { FOVRotationScale   = NewValue;  }

    // TESTS - REMOVE WHEN DONE //
    void DebugRender(CanvasWrapper Canvas);

private:
    std::shared_ptr<InputsManager> Inputs;

    //State variables set by plugin
    bool bUseOverrides;
    bool bRoll;
    float MovementSpeed;
    float MovementAccel;
    float RotationAccel;
    float MouseSensitivity;
    float GamepadSensitivity;
    float FOVRotationScale;
    
    void UpdateCameraMatrix(CameraWrapper TheCamera);
    void UpdateVelocity(float Delta);
    void UpdateAngularVelocity(float Delta);
    void UpdatePosition(float Delta, CameraWrapper TheCamera);
    void UpdateRotation(float Delta, CameraWrapper TheCamera);

    //Speed of movement and rotation
    Vector Velocity; // cm / s
    Vector AngularVelocity; // cm / s

    //Camera matrix
    Vector Forward;
    Vector Right;
    Vector Up;
};
