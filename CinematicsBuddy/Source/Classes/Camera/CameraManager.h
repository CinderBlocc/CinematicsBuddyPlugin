#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "BMGraphs/BMGraphs/BMGraphs.h"
#include <memory>

class CanvasWrapper;
class InputsManager;

class CameraManager
{
public:
    CameraManager();

    void InitCvars();

    void PlayerInputTick(float Delta, bool InbRoll);
    
    //Set state values
    void SetbUseOverrides(bool bNewValue)
    {
        bUseOverrides = bNewValue;

        // TESTS - REMOVE WHEN DONE //
        Graphs->EndRender();
        if(bUseOverrides)
        {
            GraphInitData InitData;
            InitData.Type = EGraphType::GRAPH_Line;
            InitData.Title = "Camera Relative Velocities";
            InitData.Labels = 
            {
                LabelInfo{"Forward Velocity",  LinearColor{255, 0,   0,   255}},
                LabelInfo{"Right Velocity",    LinearColor{0,   255, 0,   255}},
                LabelInfo{"Up Velocity",       LinearColor{0,   0,   255, 255}},
                LabelInfo{"Forward Input",     LinearColor{255, 180, 180, 255}},
                LabelInfo{"Right Input",       LinearColor{180, 255, 180, 255}},
                LabelInfo{"Up Input",          LinearColor{180, 180, 255, 255}}
            };
            Graphs->BeginRender(InitData);
        }
    }
    void SetbUseLocalMatrix(bool bNewValue)      { bUseLocalMatrix      = bNewValue; }
    void SetMovementSpeed(float NewValue)        { MovementSpeed        = NewValue;  }
    void SetMovementAccel(float NewValue)        { MovementAccel        = NewValue;  }
    void SetRotationSpeed(float NewValue)        { RotationSpeed        = NewValue;  }
    void SetRotationAccelMouse(float NewValue)   { RotationAccelMouse   = NewValue;  }
    void SetRotationAccelGamepad(float NewValue) { RotationAccelGamepad = NewValue;  }
    void SetMouseSensitivity(float NewValue)     { MouseSensitivity     = NewValue;  }
    void SetGamepadSensitivity(float NewValue)   { GamepadSensitivity   = NewValue;  }
    void SetFOVRotationScale(float NewValue)     { FOVRotationScale     = NewValue;  }

    // TESTS - REMOVE WHEN DONE //
    std::shared_ptr<BMGraphs> Graphs;
    void StartInputsTest();
    void DebugRender(CanvasWrapper Canvas);

private:
    std::shared_ptr<InputsManager> Inputs;

    //State variables set by plugin
    std::shared_ptr<float> TestClassCvar = std::make_shared<float>(0.f);
    bool bUseOverrides;
    bool bUseLocalMatrix;
    bool bRoll;
    float MovementSpeed;
    float MovementAccel;
    float RotationSpeed;
    float RotationAccelMouse;
    float RotationAccelGamepad;
    float MouseSensitivity;
    float GamepadSensitivity;
    float FOVRotationScale;

    //Internal state variables
    float BaseMovementSpeed;
    float BaseMovementAccel;
    float BaseRotationSpeed;
    float BaseRotationAccel;
    
    //Speed of movement and rotation
    Vector Velocity;
    Vector AngularVelocity;

    //Camera matrix
    Vector Forward;
    Vector Right;
    Vector Up;

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
