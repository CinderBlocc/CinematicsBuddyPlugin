#pragma once
#include "bakkesmod/wrappers/wrapperstructs.h"
#include <memory>
#include <string>
#include <vector>

class CameraWrapper;
class CanvasWrapper;
class InputsManager;
namespace RT
{
    class Matrix3;
}

class CameraManager
{
public:
    CameraManager();

    // TESTS - REMOVE WHEN DONE //
    std::shared_ptr<class BMGraphs> Graphs;
    void OnUseOverridesChanged();
    void StartInputsTest();
    void DebugRender(CanvasWrapper Canvas);

private:
    void PlayerInputTick();
    std::shared_ptr<InputsManager> Inputs;

    //State variables set by plugin
    std::shared_ptr<bool>  m_bUseOverrides         = std::make_shared<bool>(false);
    std::shared_ptr<bool>  m_bUseLocalMovement     = std::make_shared<bool>(true);
    std::shared_ptr<bool>  m_bUseLocalRotation     = std::make_shared<bool>(true);
    std::shared_ptr<bool>  m_bHardFloors           = std::make_shared<bool>(true);
    std::shared_ptr<float> m_FloorHeight           = std::make_shared<float>(10.f);
    std::shared_ptr<float> m_MovementSpeed         = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_MovementAccel         = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_RotationSpeed         = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_RotationAccelMouse    = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_RotationAccelGamepad  = std::make_shared<float>(1.f);
    std::shared_ptr<float> m_MouseSensitivity      = std::make_shared<float>(10.f);
    std::shared_ptr<float> m_GamepadSensitivity    = std::make_shared<float>(20.f);
    std::shared_ptr<float> m_FOVRotationScale      = std::make_shared<float>(0.9f);
    std::shared_ptr<std::string> m_RollBinding     = std::make_shared<std::string>("XboxTypeS_RightShoulder");
    int RollBindingIndex = 0;
    bool bRoll = false;
    void CacheRollBinding();

    std::string GetBindingOptions();
    std::string GetInputOptions();

    //Internal state variables
    float BaseMovementSpeed = 2000.f;
    float BaseMovementAccel = 2.f;
    float BaseRotationSpeed = 100.f;
    float BaseRotationAccel = 2.f;
    
    //Speed of movement and rotation
    Vector Velocity        = {0, 0, 0};
    Vector AngularVelocity = {0, 0, 0};

    //Functions to update camera transformation
    void UpdateCameraTransformation(float Delta);
    void UpdateVelocity(float Delta, RT::Matrix3 MovementMatrix);
    void UpdateAngularVelocity(float Delta, RT::Matrix3 RotationMatrix);
    void UpdatePosition(float Delta, CameraWrapper TheCamera);
    void UpdateRotation(float Delta, CameraWrapper TheCamera);

    //Utility
    bool  IsValidMode();
    float GetDelta();
    RT::Matrix3 GetCameraMatrix(bool bFullyLocal);
    float GetSpeedComponent(Vector Direction);
    float GetAngularSpeedComponent(Vector Direction);
    float GetInvertedPerc(float InPerc);
    float GetWeightedPerc(float InPerc);
    float GetReducedPerc(float InputPerc, float SpeedPerc);
    float GetBrakeForce(float InputPerc, float SpeedPerc);
    Quat  AngleAxisRotation(float angle, Vector axis);
    float RemapPercentage(float CurrentPerc, float CurrentMin, float CurrentMax, float NewMin, float NewMax); //#TODO: Remove this? Is it used anywhere?
};
