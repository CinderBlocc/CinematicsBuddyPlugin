#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "BMGraphs/BMGraphs/BMGraphs.h"
#include <memory>

class CanvasWrapper;
class InputsManager;

class CameraManager
{
public:
    void InitCvars();
    void PlayerInputTick(float Delta);

    // TESTS - REMOVE WHEN DONE //
    std::shared_ptr<BMGraphs> Graphs;
    void OnUseOverridesChanged();
    void StartInputsTest();
    void DebugRender(CanvasWrapper Canvas);

private:
    std::shared_ptr<InputsManager> Inputs;

    //State variables set by plugin
    std::shared_ptr<bool>  m_bUseOverrides         = std::make_shared<bool>(false);
    std::shared_ptr<bool>  m_bUseLocalMatrix       = std::make_shared<bool>(true);
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
    void RecacheRollBinding();

    //Internal state variables
    float BaseMovementSpeed = 2000.f;
    float BaseMovementAccel = 2.f;
    float BaseRotationSpeed = 1.f;
    float BaseRotationAccel = 1.f;
    
    //Speed of movement and rotation
    Vector Velocity        = {0, 0, 0};
    Vector AngularVelocity = {0, 0, 0};

    //Camera matrix
    Vector Forward = {1, 0, 0};
    Vector Right   = {0, 1, 0};
    Vector Up      = {0, 0, 1};

    //Functions to update camera transformation
    void UpdateCameraTransformation(float Delta);
    void UpdateCameraMatrix(CameraWrapper TheCamera);
    void UpdateVelocity(float Delta);
    void UpdateAngularVelocity(float Delta);
    void UpdatePosition(float Delta, CameraWrapper TheCamera);
    void UpdateRotation(float Delta, CameraWrapper TheCamera);

    //Utility
    float GetSpeedComponent(Vector Direction);
    float GetInvertedPerc(float InPerc);
    float GetWeightedPerc(float InPerc);
    float GetReducedPerc(float InputPerc, float SpeedPerc);
    float GetBrakeForce(float InputPerc, float SpeedPerc);
    float RemapPercentage(float CurrentPerc, float CurrentMin, float CurrentMax, float NewMin, float NewMax); //#TODO: Remove this? Is it used anywhere?
};
