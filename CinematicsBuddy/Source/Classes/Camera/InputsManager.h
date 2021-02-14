#pragma once
#include "bakkesmod/wrappers/wrapperstructs.h"

class InputsManager
{
public:
    InputsManager();

    void PlayerInputTick(float Delta);
    void ResetValues();

    void DebugRender(class CanvasWrapper Canvas);

    bool bUsingGamepad;
    bool bRoll;
    float MouseSensitivity;
    float Acceleration;
    float Speed;
    float Forward;
    float Strafe;
    float Up;
    float Pitch;//LookUp
    float Yaw;//Turn
    float Roll;//Keyboard: LookRoll | Gamepad: bRoll + Turn (Yaw)
    float LookRightScale;
    float LookUpScale;
    float GamepadLookScale;

private:
    bool bDirty;
    void GetInputs(PlayerControllerWrapper Controller);
    void NullifyInputs(PlayerControllerWrapper Controller);
    void UpdateVelocity(float Delta);
    void UpdateAngularVelocity(float Delta);

    Vector Velocity;
    Vector AngularVelocity;
    Vector CurrentLocation;
    Vector CurrentRotation;
    Vector PreviousLocation;
    Vector PreviousRotation;
};
