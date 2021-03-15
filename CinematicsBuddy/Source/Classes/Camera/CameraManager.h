#pragma once
#include "bakkesmod/wrappers/wrapperstructs.h"
#include <memory>
#include <string>
#include <vector>

class CameraConfigManager;
class CameraWrapper;
class CanvasWrapper;
class InputsManager;
namespace RT
{
    class Matrix3;
}

enum class ECameraAxis
{
    AForward = 0,
    ARight,
    AUp
};

class CameraManager
{
public:
    CameraManager();

private:
    void PlayerInputTick();
    std::shared_ptr<InputsManager> Inputs;
    std::shared_ptr<CameraConfigManager> Configs;

    //State variables set by plugin
    std::shared_ptr<bool>  bUseOverrides        = std::make_shared<bool>(false);
    std::shared_ptr<bool>  bUseLocalMovement    = std::make_shared<bool>(true);
    std::shared_ptr<bool>  bUseLocalRotation    = std::make_shared<bool>(false);
    std::shared_ptr<bool>  bHardFloors          = std::make_shared<bool>(true);
    std::shared_ptr<bool>  bLocalMomentum       = std::make_shared<bool>(false);
    std::shared_ptr<float> FloorHeight          = std::make_shared<float>(10.f);
    std::shared_ptr<float> MovementSpeed        = std::make_shared<float>(1.f);
    std::shared_ptr<float> MovementAccel        = std::make_shared<float>(1.f);
    std::shared_ptr<float> RotationSpeedMouse   = std::make_shared<float>(1.f);
    std::shared_ptr<float> RotationSpeedGamepad = std::make_shared<float>(1.f);
    std::shared_ptr<float> RotationAccelMouse   = std::make_shared<float>(1.f);
    std::shared_ptr<float> RotationAccelGamepad = std::make_shared<float>(1.f);
    std::shared_ptr<float> FOVRotationScale     = std::make_shared<float>(.3f);
    std::shared_ptr<float> FOVMin               = std::make_shared<float>(20.f);
    std::shared_ptr<float> FOVMax               = std::make_shared<float>(120.f);
    std::shared_ptr<float> FOVSpeed             = std::make_shared<float>(1.f);
    std::shared_ptr<float> FOVAcceleration      = std::make_shared<float>(1.f);
    std::shared_ptr<float> FOVLimitEase         = std::make_shared<float>(.1f);

    //Internal state variables
    float BaseMovementSpeed = 1500.f;
    float BaseMovementAccel = 2.f;
    float BaseRotationSpeedMouse   = 200.f;
    float BaseRotationSpeedGamepad = 100.f;
    float BaseRotationAccelMouse   = 3.f;
    float BaseRotationAccelGamepad = 2.f;
    float BaseFOV      = 75.f;
    float BaseFOVSpeed = 40.f;
    float BaseFOVAccel = 2.f;
    
    //Speed of movement and rotation
    Vector VelocityWorld   = {0, 0, 0};
    Vector VelocityLocal   = {0, 0, 0};
    Vector AngularVelocity = {0, 0, 0};
    float  FOVCurrentSpeed = 0.f;

    //Functions to update camera transformation
    void UpdateCamera(float Delta);
    void UpdatePosition(float Delta, CameraWrapper TheCamera);
    void UpdateRotation(float Delta, CameraWrapper TheCamera);
    void UpdateFOV(float Delta, CameraWrapper TheCamera);

    //Speed calculations
    void ResetSpeeds();
    void UpdateVelocityLocal(float Delta, Vector MovementInputs);
    void UpdateVelocityWorld(float Delta, Vector MovementInputs);
    void UpdateAngularVelocity(float Delta, Vector RotationInputs);
    void UpdateFOVSpeed(float Delta);
    float UpdateFloatSpeed(float Delta, float CurrentSpeed, float Input, float MaxSpeed, float AccelSpeed);

    //Inputs
    Vector GetMovementInputs();
    Vector GetRotationInputs();

    //Utility
    bool  IsValidMode();
    float GetDelta();
    RT::Matrix3 GetCameraMatrix(bool bFullyLocal, bool bLocationMatrix);
    float GetInvertedPerc(float InPerc);
    float GetWeightedPerc(float InPerc);
    float GetReducedPerc(float InputPerc, float SpeedPerc);
    float GetBrakeForce(float InputPerc, float SpeedPerc);
    Quat  AngleAxisRotation(float angle, Vector axis);
    float GetFOVScaleReduction();
    float GetClampedFOV(float InCurrentFOV, float Min, float Max);
    float RemapPercentage(float CurrentPerc, float CurrentMin, float CurrentMax, float NewMin, float NewMax);
};
