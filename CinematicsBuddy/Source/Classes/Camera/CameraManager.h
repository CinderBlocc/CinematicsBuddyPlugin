#pragma once
#include "bakkesmod/wrappers/wrapperstructs.h"
#include <memory>
#include <string>
#include <vector>

class CameraConfigManager;
class CameraWrapper;
class CanvasWrapper;
class InputsManager;
class UIManager;
namespace RT
{
    class Matrix3;
}

class CameraManager
{
public:
    CameraManager(std::shared_ptr<UIManager> TheUI);

    // TESTS - REMOVE WHEN DONE //
    void DebugRender(CanvasWrapper Canvas);

private:
    void PlayerInputTick();
    std::shared_ptr<UIManager> UI;
    std::shared_ptr<InputsManager> Inputs;
    std::shared_ptr<CameraConfigManager> Configs;

    //State variables set by plugin
    std::shared_ptr<bool>  m_bUseOverrides        = std::make_shared<bool>(false);
    std::shared_ptr<bool>  m_bUseLocalMovement    = std::make_shared<bool>(true);
    std::shared_ptr<bool>  m_bUseLocalRotation    = std::make_shared<bool>(false);
    std::shared_ptr<bool>  m_bHardFloors          = std::make_shared<bool>(true);
    std::shared_ptr<bool>  m_bLocalMomentum       = std::make_shared<bool>(true);
    std::shared_ptr<float> m_FloorHeight          = std::make_shared<float>(10.f);
    std::shared_ptr<float> m_MovementSpeed        = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_MovementAccel        = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_RotationSpeed        = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_RotationAccelMouse   = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_RotationAccelGamepad = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_MouseSensitivity     = std::make_shared<float>(10.f);
    std::shared_ptr<float> m_GamepadSensitivity   = std::make_shared<float>(20.f);
    std::shared_ptr<float> m_FOVRotationScale     = std::make_shared<float>(.3f);
    std::shared_ptr<float> m_FOVMin               = std::make_shared<float>(20.f);
    std::shared_ptr<float> m_FOVMax               = std::make_shared<float>(120.f);
    std::shared_ptr<float> m_FOVSpeed             = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_FOVAcceleration      = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_FOVLimitEase         = std::make_shared<float>(.1f);

    //Internal state variables
    float BaseMovementSpeed = 1500.f;
    float BaseMovementAccel = 2.f;
    float BaseRotationSpeed = 100.f;
    float BaseRotationAccel = 2.f;
    float BaseFOV      = 75.f;
    float BaseFOVSpeed = 35.f;
    float BaseFOVAccel = 2.f;
    
    //Speed of movement and rotation
    Vector Velocity        = {0, 0, 0};
    Vector AngularVelocity = {0, 0, 0};
    float  FOVSpeed        = 0.f;

    //Functions to update camera transformation
    void UpdateCamera(float Delta);
    void UpdateVelocity(float Delta, RT::Matrix3 MovementMatrix);
    void UpdateAngularVelocity(float Delta, RT::Matrix3 RotationMatrix);
    void UpdateFOVSpeed(float Delta);
    void UpdatePosition(float Delta, CameraWrapper TheCamera);
    void UpdateRotation(float Delta, CameraWrapper TheCamera);
    void UpdateFOV(float Delta, CameraWrapper TheCamera);

    //Utility
    bool  IsValidMode();
    float GetDelta();
    RT::Matrix3 GetCameraMatrix(bool bFullyLocal);
    float GetSpeedComponent(Vector Direction);
    float GetAngularSpeedComponent(Vector Direction, float FOVScaleReduction);
    float GetInvertedPerc(float InPerc);
    float GetWeightedPerc(float InPerc);
    float GetReducedPerc(float InputPerc, float SpeedPerc);
    float GetBrakeForce(float InputPerc, float SpeedPerc);
    Quat  AngleAxisRotation(float angle, Vector axis);
    float GetFOVScaleReduction();
    float GetClampedFOV(float InCurrentFOV, float Min, float Max);
    float RemapPercentage(float CurrentPerc, float CurrentMin, float CurrentMax, float NewMin, float NewMax);
};
