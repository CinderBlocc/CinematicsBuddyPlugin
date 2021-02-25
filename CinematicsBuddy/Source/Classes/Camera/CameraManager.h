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
    std::shared_ptr<bool>  m_bLocalMomentum       = std::make_shared<bool>(false);
    std::shared_ptr<float> m_FloorHeight          = std::make_shared<float>(10.f);
    std::shared_ptr<float> m_MovementSpeed        = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_MovementAccel        = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_RotationSpeedMouse   = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_RotationSpeedGamepad = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_RotationAccelMouse   = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_RotationAccelGamepad = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_FOVRotationScale     = std::make_shared<float>(.3f);
    std::shared_ptr<float> m_FOVMin               = std::make_shared<float>(20.f);
    std::shared_ptr<float> m_FOVMax               = std::make_shared<float>(120.f);
    std::shared_ptr<float> m_FOVSpeed             = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_FOVAcceleration      = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_FOVLimitEase         = std::make_shared<float>(.1f);

    //Internal state variables
    float BaseMovementSpeed = 1500.f;
    float BaseMovementAccel = 2.f;
    float BaseRotationSpeedMouse   = 200.f;
    float BaseRotationSpeedGamepad = 100.f;
    float BaseRotationAccelMouse   = 3.f;
    float BaseRotationAccelGamepad = 2.f;
    float BaseFOV      = 75.f;
    float BaseFOVSpeed = 35.f;
    float BaseFOVAccel = 2.f;
    
    //Speed of movement and rotation
    Vector VelocityWorld   = {0, 0, 0};
    Vector VelocityLocal   = {0, 0, 0};
    Vector AngularVelocity = {0, 0, 0};
    float  FOVSpeed        = 0.f;

    //Functions to update camera transformation
    void UpdateCamera(float Delta);
    void UpdateVelocityLocal(float Delta);
    void UpdateVelocityWorld(float Delta, RT::Matrix3 MovementMatrix);
    void UpdateAngularVelocity(float Delta);
    Vector2F GetMousePitchYaw(float Delta);
    Vector2F GetGamepadPitchYaw(float Delta);
    float GetRoll(float Delta);
    void UpdateFOVSpeed(float Delta);
    void UpdatePosition(float Delta, CameraWrapper TheCamera, RT::Matrix3 MovementMatrix);
    void UpdateRotation(float Delta, CameraWrapper TheCamera, RT::Matrix3 RotationMatrix);
    void UpdateFOV(float Delta, CameraWrapper TheCamera);

    //Utility
    bool  IsValidMode();
    float GetDelta();
    RT::Matrix3 GetCameraMatrix(bool bFullyLocal, bool bLocationMatrix);
    float GetWorldSpeedComponent(Vector Direction);
    float GetInvertedPerc(float InPerc);
    float GetWeightedPerc(float InPerc);
    float GetReducedPerc(float InputPerc, float SpeedPerc);
    float GetBrakeForce(float InputPerc, float SpeedPerc);
    Quat  AngleAxisRotation(float angle, Vector axis);
    float GetFOVScaleReduction();
    float GetClampedFOV(float InCurrentFOV, float Min, float Max);
    float RemapPercentage(float CurrentPerc, float CurrentMin, float CurrentMax, float NewMin, float NewMax);
};
