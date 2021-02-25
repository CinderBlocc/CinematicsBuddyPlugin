#include "CameraManager.h"
#include "InputsManager.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "RenderingTools/RenderingTools.h"
#include "CameraConfigManager.h"
#include "UI/UIManager.h"

/*

    #TODO:
        - When using non-local rotation and rolled, pitching will eventually reset roll back to 0
            - Maybe forward.Z shouldnt be set to 0, only right.Z should be?
            - GetAngularComponent might be calculated wrong
            - Simplest solution (should also be applied to local linear momentum preservation):
                Store the Pitch, Yaw, and Roll speeds as separate floats, then just apply those to the direction the axis is currently pointing
                    - In the UpdateVelocity and UpdateAngularVelocity functions, only apply the acceleration to those floats

        - Add mouse input

        - Instead of a cvar for inverting pitch, use the game's settings?

        - Create a "Normal" preset that roughly matches the way the game normally does camera inputs

        - Fine tune default values until they feel the most user friendly, then save those as a "Default" preset

*/

CameraManager::CameraManager(std::shared_ptr<UIManager> TheUI)
{
    UI = TheUI;
    Inputs = std::make_shared<InputsManager>(TheUI);

    //Register cvars
    UI->AddElement({m_bUseOverrides,        CVAR_ENABLE_CAM_OVERRIDE, "Enable Overrides",                   "Enables camera overriding features"                            });
    UI->AddElement({m_bUseLocalMovement,    CVAR_CAM_LOCAL_MOVEMENT,  "Local movement",                     "Uses the local orientation of the camera for movement"         });
    UI->AddElement({m_bUseLocalRotation,    CVAR_CAM_LOCAL_ROTATION,  "Local rotation",                     "Uses the local orientation of the camera for rotation"         });
    UI->AddElement({m_bHardFloors,          CVAR_CAM_HARD_FLOORS,     "Hard floors",                        "Prevents the camera from going through the floor"              });
    UI->AddElement({m_bLocalMomentum,       CVAR_CAM_LOCAL_MOMENTUM,  "Momentum is local",                  "Preserves momentum in local space instead of world space"      });
    UI->AddElement({m_FloorHeight,          CVAR_CAM_FLOOR_HEIGHT,    "Floor Height",                       "Lowest height the camera can go",                      -50, 50 });
    UI->AddElement({m_MovementSpeed,        CVAR_CAM_MOVEMENT_SPEED,  "Movement Speed",                     "Camera movement speed multiplier",                      0,  5  });
    UI->AddElement({m_MovementAccel,        CVAR_CAM_MOVEMENT_ACCEL,  "Movement Acceleration",              "Camera movement acceleration speed",                    0,  5  });
	UI->AddElement({m_RotationSpeed,        CVAR_ROT_SPEED,           "Rotation Speed (non-mouse)",         "Camera rotation speed (non-mouse)",                     0,  3  });
    UI->AddElement({m_RotationAccelMouse,   CVAR_ROT_ACCEL_MOUSE,     "Rotation Acceleration (Mouse)",      "Camera rotation acceleration speed (mouse)",            0,  10 });
    UI->AddElement({m_RotationAccelGamepad, CVAR_ROT_ACCEL_GAMEPAD,   "Rotation Acceleration (Controller)", "Camera rotation acceleration speed (controller)",       0,  10 });
    UI->AddElement({m_MouseSensitivity,     CVAR_MOUSE_SENSITIVITY,   "Mouse Sensitivity",                  "Camera rotation speed when using mouse",                0,  25 });
    UI->AddElement({m_GamepadSensitivity,   CVAR_GAMEPAD_SENSITIVITY, "Gamepad Sensitivity",                "Camera rotation speed when using gamepad",              0,  50 });
    UI->AddElement({m_FOVRotationScale,     CVAR_FOV_ROTATION_SCALE,  "FOV Rotation Scale",                 "Multiplier for slowing camera rotation when zoomed in", 0,  1  });
    UI->AddElement({m_FOVMin,               CVAR_FOV_MIN,             "FOV Minimum",                        "The lowest the FOV will go",                            5,  170});
    UI->AddElement({m_FOVMax,               CVAR_FOV_MAX,             "FOV Maximum",                        "The highest the FOV will go",                           5,  170});
    UI->AddElement({m_FOVSpeed,             CVAR_FOV_SPEED,           "FOV Speed",                          "The speed at which FOV will change",                    0,  3  });
    UI->AddElement({m_FOVAcceleration,      CVAR_FOV_ACCELERATION,    "FOV Acceleration",                   "The easing of FOV speed change",                        0,  10 });
    UI->AddElement({m_FOVLimitEase,         CVAR_FOV_LIMIT_EASE,      "FOV Limit Ease",                     "Cutoff for easing effect into min/max FOV",             0,  .5f});

    //Register hooks
    GlobalGameWrapper->HookEvent("Function TAGame.PlayerInput_TA.PlayerInput", std::bind(&CameraManager::PlayerInputTick, this));

    //Create the config manager after all cvars have been created
    Configs = std::make_shared<CameraConfigManager>(TheUI);
}

void CameraManager::PlayerInputTick()
{
    //Get delta regardless of validity so there isn't suddenly a massive jump when it becomes valid again
    float Delta = GetDelta();

    if(!IsValidMode())
    {
        return;
    }

    //Get the inputs and then nullify them before they reach the game
	Inputs->PlayerInputTick(Delta);

    //Apply inputs to camera
    UpdateCamera(Delta);
}

void CameraManager::UpdateCamera(float Delta)
{
    CameraWrapper TheCamera = GlobalGameWrapper->GetCamera();
    if(!TheCamera.IsNull())
    {
        RT::Matrix3 MovementMatrix = GetCameraMatrix(*m_bUseLocalMovement, true);
        RT::Matrix3 RotationMatrix = GetCameraMatrix(*m_bUseLocalRotation, false);
        UpdateVelocityLocal(Delta, MovementMatrix);
        UpdateVelocityWorld(Delta, MovementMatrix);
        UpdateAngularVelocity(Delta, RotationMatrix);
        UpdateFOVSpeed(Delta);
        UpdatePosition(Delta, TheCamera, MovementMatrix);
        UpdateRotation(Delta, TheCamera);
        UpdateFOV(Delta, TheCamera);
    }
}

// SPEED CALCULATIONS //
void CameraManager::UpdateVelocityLocal(float Delta, RT::Matrix3 MovementMatrix)
{
    //Calculate some variables used throughout the function
    float MaxSpeed = BaseMovementSpeed * *m_MovementSpeed;
    float AccelSpeed = BaseMovementAccel * *m_MovementAccel;
    float ImpulseStrength = MaxSpeed * Delta * AccelSpeed;

    //Get the camera's speed as a percentage per axis
    float ForwardSpeedPerc = VelocityLocal.X / MaxSpeed;
    float RightSpeedPerc   = VelocityLocal.Y / MaxSpeed;
    float UpSpeedPerc      = VelocityLocal.Z / MaxSpeed;

    //Get input percentages
    float ForwardInputPerc = Inputs->GetForward();
    float RightInputPerc   = Inputs->GetRight();
    float UpInputPerc      = Inputs->GetUp();

    //Calculate acceleration force per axis
    float ForwardAcceleration = ForwardInputPerc * GetReducedPerc(ForwardInputPerc, ForwardSpeedPerc) * ImpulseStrength;
    float RightAcceleration   = RightInputPerc   * GetReducedPerc(RightInputPerc,   RightSpeedPerc)   * ImpulseStrength;
    float UpAcceleration      = UpInputPerc      * GetReducedPerc(UpInputPerc,      UpSpeedPerc)      * ImpulseStrength;

    //Calculate automatic braking force (only apply brakes if no input on that axis)
    float ForwardBrake = GetBrakeForce(ForwardInputPerc, ForwardSpeedPerc) * ImpulseStrength;
    float RightBrake   = GetBrakeForce(RightInputPerc,   RightSpeedPerc)   * ImpulseStrength;
    float UpBrake      = GetBrakeForce(UpInputPerc,      UpSpeedPerc)      * ImpulseStrength;

    //Store the individual speeds in each axis of the VelocityLocal variable
    VelocityLocal.X += ForwardAcceleration - ForwardBrake;
    VelocityLocal.Y += RightAcceleration   - RightBrake;
    VelocityLocal.Z += UpAcceleration      - UpBrake;
}

void CameraManager::UpdateVelocityWorld(float Delta, RT::Matrix3 InMatrix)
{
    //Calculate some variables used throughout the function
    float MaxSpeed = BaseMovementSpeed * *m_MovementSpeed;
    float AccelSpeed = BaseMovementAccel * *m_MovementAccel;
    float ImpulseStrength = MaxSpeed * Delta * AccelSpeed;

    //Get the camera's speed as a percentage per axis
    float ForwardSpeedPerc = GetSpeedComponent(InMatrix.forward);
    float RightSpeedPerc   = GetSpeedComponent(InMatrix.right);
    float UpSpeedPerc      = GetSpeedComponent(InMatrix.up);

    //Get input percentages
    float ForwardInputPerc = Inputs->GetForward();
    float RightInputPerc   = Inputs->GetRight();
    float UpInputPerc      = Inputs->GetUp();

    //Convert those inputs into vector inputs
    Vector ForwardInputVec = InMatrix.forward * ForwardInputPerc;
    Vector RightInputVec   = InMatrix.right   * RightInputPerc;
    Vector UpInputVec      = InMatrix.up      * UpInputPerc;

    //Calculate acceleration force per axis
    Vector ForwardAcceleration = ForwardInputVec * GetReducedPerc(ForwardInputPerc, ForwardSpeedPerc);
    Vector RightAcceleration   = RightInputVec   * GetReducedPerc(RightInputPerc,   RightSpeedPerc);
    Vector UpAcceleration      = UpInputVec      * GetReducedPerc(UpInputPerc,      UpSpeedPerc);
    
    //Calculate impulse created by inputs
    Vector AccelerationDirection = ForwardAcceleration + RightAcceleration + UpAcceleration;
    Vector Acceleration = AccelerationDirection * ImpulseStrength;

    //Calculate automatic braking force (only apply brakes if no input on that axis)
    Vector ForwardBrake = InMatrix.forward * GetBrakeForce(ForwardInputPerc, ForwardSpeedPerc);
    Vector RightBrake   = InMatrix.right   * GetBrakeForce(RightInputPerc,   RightSpeedPerc);
    Vector UpBrake      = InMatrix.up      * GetBrakeForce(UpInputPerc,      UpSpeedPerc);
    
    //Calculate impulse created by braking
    Vector BrakeDirection = ForwardBrake + RightBrake + UpBrake;
    Vector Brake = BrakeDirection * ImpulseStrength;

    //Apply the impulses
    VelocityWorld += Acceleration - Brake;
}

void CameraManager::UpdateAngularVelocity(float Delta, RT::Matrix3 InMatrix)
{
    //RotationSpeed should only be taken into account for Pitch and Yaw if bUsingGamepad is true
    //Roll is set by both keyboard and controller as a rate, along with Pitch and Yaw on controller
    //Pitch and Yaw are set by mouse via movement deltas which give large numbers, so speed should not be used

    //Calculate some variables used throughout the function
    float MaxSpeed = BaseRotationSpeed * *m_RotationSpeed;
    float MouseAccelSpeed   = BaseRotationAccel * *m_RotationAccelMouse;
    float GamepadAccelSpeed = BaseRotationAccel * *m_RotationAccelGamepad;
    float GamepadImpulseStrength = MaxSpeed * Delta * GamepadAccelSpeed;
    float MouseImpulseStrength   = MaxSpeed * Delta * MouseAccelSpeed;

    //Get the camera's angular speed as a percentage per axis
    float FOVScaleReduction = GetFOVScaleReduction();
    float PitchSpeedPerc    = GetAngularSpeedComponent(InMatrix.right);
    float YawSpeedPerc      = GetAngularSpeedComponent(InMatrix.up);
    float RollSpeedPerc     = GetAngularSpeedComponent(InMatrix.forward);

    //Get input percentages
    float PitchInputPerc = 0.f;
    float YawInputPerc = 0.f;
    float RollInputPerc = Inputs->GetRoll() * -1.f;
    if(Inputs->GetbUsingGamepad())
    {
        PitchInputPerc = Inputs->GetPitch() * -1.f;
        YawInputPerc   = Inputs->GetYaw();
    }
    else
    {
        //#TODO: Add mouse input
    }

    //Convert those inputs into vector inputs
    Vector PitchInputVec = InMatrix.right   * PitchInputPerc;
    Vector YawInputVec   = InMatrix.up      * YawInputPerc;
    Vector RollInputVec  = InMatrix.forward * RollInputPerc;

    //Calculate acceleration force per axis
    //float FinalImpulseStrength = Inputs->GetbUsingGamepad() ? GamepadImpulseStrength : MouseImpulseStrength;
    float FinalImpulseStrength = GamepadImpulseStrength;
    float FOVScaleImpulseStrength = FinalImpulseStrength * FOVScaleReduction;
    Vector PitchAcceleration = PitchInputVec * GetReducedPerc(PitchInputPerc, PitchSpeedPerc) * FOVScaleImpulseStrength;
    Vector YawAcceleration   = YawInputVec   * GetReducedPerc(YawInputPerc,   YawSpeedPerc)   * FOVScaleImpulseStrength;
    Vector RollAcceleration  = RollInputVec  * GetReducedPerc(RollInputPerc,  RollSpeedPerc)  * FinalImpulseStrength;

    //Calculate impulse created by inputs
    Vector AccelerationDirection = PitchAcceleration + YawAcceleration + RollAcceleration;
    Vector Acceleration = AccelerationDirection;// * GamepadImpulseStrength;

    //Calculate automatic braking force (only apply brakes if no input on that axis)
    Vector PitchBrake = InMatrix.right   * GetBrakeForce(PitchInputPerc, PitchSpeedPerc);
    Vector YawBrake   = InMatrix.up      * GetBrakeForce(YawInputPerc,   YawSpeedPerc);
    Vector RollBrake  = InMatrix.forward * GetBrakeForce(RollInputPerc,  RollSpeedPerc);
    
    //Calculate impulse created by braking
    Vector BrakeDirection = PitchBrake + YawBrake + RollBrake;
    Vector Brake = BrakeDirection * GamepadImpulseStrength;

    //Apply the impulses
    AngularVelocity += Acceleration - Brake;
}

void CameraManager::UpdateFOVSpeed(float Delta)
{
    //Calculate some variables used throughout the function
    float MaxSpeed = BaseFOVSpeed * *m_FOVSpeed;
    float AccelSpeed = BaseFOVAccel * *m_FOVAcceleration;
    float ImpulseStrength = MaxSpeed * Delta * AccelSpeed;

    //Get speed percentage
    float SpeedPerc = FOVSpeed / MaxSpeed;

    //Get input percentage
    float InputPerc = Inputs->GetFOV() * -1.f;

    //Calculate acceleration
    float Acceleration = InputPerc * GetReducedPerc(InputPerc, SpeedPerc);
    Acceleration *= ImpulseStrength;

    //Calculate brake
    float Brake = GetBrakeForce(InputPerc, SpeedPerc);
    Brake *= ImpulseStrength;

    //Apply the impulses
    FOVSpeed += Acceleration - Brake;
}

// SPEED RESULT CALCULATIONS //
void CameraManager::UpdatePosition(float Delta, CameraWrapper TheCamera, RT::Matrix3 MovementMatrix)
{
    Vector NewLocation = TheCamera.GetLocation();
    
    if(*m_bLocalMomentum)
    {
        NewLocation += MovementMatrix.forward * VelocityLocal.X * Delta;
        NewLocation += MovementMatrix.right   * VelocityLocal.Y * Delta;
        NewLocation += MovementMatrix.up      * VelocityLocal.Z * Delta;
    }
    else
    {
        NewLocation += VelocityWorld * Delta;
    }

    if(*m_bHardFloors)
    {
        NewLocation.Z = max(NewLocation.Z, *m_FloorHeight);
    }

    TheCamera.SetLocation(NewLocation);
}

void CameraManager::UpdateRotation(float Delta, CameraWrapper TheCamera)
{
    RT::Matrix3 CurrentMatrix(TheCamera.GetRotation());

    //Only apply new rotation if it is non-zero
    float RotationAmount = AngularVelocity.magnitude() * Delta * (CONST_PI_F / 180.f);
    if(abs(RotationAmount) >= 0.00001f)
    {
        Vector RotationAxis = AngularVelocity.getNormalized();
        Quat NewRotation = AngleAxisRotation(RotationAmount, RotationAxis);
        CurrentMatrix.RotateWithQuat(NewRotation);
    }
    
    TheCamera.SetRotation(CurrentMatrix.ToRotator());
}

void CameraManager::UpdateFOV(float Delta, CameraWrapper TheCamera)
{
    float Min = *m_FOVMin < *m_FOVMax ? *m_FOVMin : *m_FOVMax;
    float Max = *m_FOVMin < *m_FOVMax ? *m_FOVMax : *m_FOVMin;

    float CurrentFOV = TheCamera.GetFOV();
    float FOVPerc = RemapPercentage(CurrentFOV, Min, Max, 0.f, 1.f);
    bool bInLowerLimitRange = FOVPerc <= *m_FOVLimitEase;
    bool bInUpperLimitRange = FOVPerc >= 1.f - *m_FOVLimitEase;
    float ReductionStrength = 1.f;
    if(bInLowerLimitRange || bInUpperLimitRange)
    {
        //FOV is approaching limits. If FOVSpeed is continuing to head in that direction, reduce its strength
        //Since this is used as a multiplier, 1 is no reduction, 0 is full reduction
        if(bInLowerLimitRange && FOVSpeed < 0.f)
        {
            ReductionStrength = RemapPercentage(FOVPerc, 0.f, *m_FOVLimitEase, 0.f, 1.f);
        }
        else if(bInUpperLimitRange && FOVSpeed > 0.f)
        {
            ReductionStrength = 1.f - RemapPercentage(FOVPerc, 1.f - *m_FOVLimitEase, 1.f, 0.f, 1.f);
        }
    }

    float NewFOV = TheCamera.GetFOV() + FOVSpeed * Delta * ReductionStrength;
    NewFOV = GetClampedFOV(NewFOV, Min, Max);
    TheCamera.SetFOV(NewFOV);
}


// UTILITY //
bool CameraManager::IsValidMode()
{
    //#TODO: Check if camera is spectator or something? Check only if they're in replay?
    //Test to make sure they can't move camera around while demolished

    if(!*m_bUseOverrides)
    {
        return false;
    }

    if(!GlobalGameWrapper->IsInReplay())
    {
        return false;
    }

    return true;
}

float CameraManager::GetDelta()
{
    using namespace std::chrono;

    //PreviousTime is "static" so it is only created and initialized once
    static steady_clock::time_point PreviousTime = steady_clock::now();
    
    //Store the current time and calculate the delta from that
    steady_clock::time_point CurrentTime = steady_clock::now();
    float InputDelta = duration_cast<duration<float>>(CurrentTime - PreviousTime).count();
    
    //Set PreviousTime for the next delta call
    PreviousTime = CurrentTime;

    return InputDelta;
}

RT::Matrix3 CameraManager::GetCameraMatrix(bool bFullyLocal, bool bLocationMatrix)
{
    CameraWrapper TheCamera = GlobalGameWrapper->GetCamera();
    if(TheCamera.IsNull())
    {
        return RT::Matrix3();
    }

    //Return the matrix constructed from the camera's current orientation
    if(bFullyLocal)
    {
        return RT::Matrix3(TheCamera.GetRotation());
    }

    //Approximate the game's camera matrix. Local pitch and roll, world yaw
    RT::Matrix3 Output(TheCamera.GetRotation());
    if(bLocationMatrix)
    {
        Output.forward.Z = 0.f; Output.forward.normalize();
    }
    Output.right.Z   = 0.f; Output.right.normalize();
    Output.up = Vector(0, 0, 1);

    return Output;
}

float CameraManager::GetSpeedComponent(Vector Direction)
{
    float MaxSpeed = BaseMovementSpeed * *m_MovementSpeed;
    return Vector::dot(VelocityWorld, Direction) / MaxSpeed;
}

float CameraManager::GetAngularSpeedComponent(Vector Direction)
{
    float MaxSpeed = BaseRotationSpeed * *m_RotationSpeed;
    return Vector::dot(AngularVelocity, Direction) / MaxSpeed;
}

float CameraManager::GetInvertedPerc(float InPerc)
{
    return (InPerc >= 0.f) ? (1.f - InPerc) : (1.f + InPerc);
}

float CameraManager::GetWeightedPerc(float InPerc)
{
    //return InPerc;

    //Some ease functions don't like negative values
    //Preserve the negative status for later and use absolute value
    float NegativeMult = InPerc < 0.f ? -1.f : 1.f;
    InPerc = abs(InPerc);

    //Ease-out
    float EasedPerc = sin((InPerc * CONST_PI_F) / 2.f);

    //Ease-in ease-out
    //float EasedPerc = -(cos(InPerc * CONST_PI_F) - 1.f) / 2.f;

    return EasedPerc * NegativeMult;
}

float CameraManager::GetReducedPerc(float InputPerc, float SpeedPerc)
{
    //If the input is applying more force in the current direction of speed,
    //reduce the force as it approaches max

    float Output = 1.f;

    if((SpeedPerc >= 0.f && InputPerc >= 0.f) || (SpeedPerc <= 0.f && InputPerc <= 0.f))
    {
        Output = GetInvertedPerc(SpeedPerc); 
    }

    return GetWeightedPerc(Output);
}

float CameraManager::GetBrakeForce(float InputPerc, float SpeedPerc)
{
    if(abs(InputPerc) < 0.001f)
    {
        return GetWeightedPerc(SpeedPerc);
    }

    return 0.f;
}

Quat CameraManager::AngleAxisRotation(float angle, Vector axis)
{
	//Angle in radians
	Quat result;
	float angDiv2 = angle * 0.5f;
	result.W = cosf(angDiv2);
    result.X = axis.X * sinf(angDiv2);
    result.Y = axis.Y * sinf(angDiv2);
    result.Z = axis.Z * sinf(angDiv2);

	return result;
}

float CameraManager::GetFOVScaleReduction()
{
    //Returns range between *m_FOVRotationScale and 1
    //Full reduction effect at *m_FOVRotationScale
    //No reduction effect at 1

    CameraWrapper TheCamera = GlobalGameWrapper->GetCamera();
    if(TheCamera.IsNull()) { return 1.f; }

    //Determine how far below the BaseFOV threshold the camera is
    //Everything above BaseFOV will be 1, everything below down to Min is less than 1
    //Min is 0
    float Min = *m_FOVMin < *m_FOVMax ? *m_FOVMin : *m_FOVMax;
    float FOVPerc = RemapPercentage(TheCamera.GetFOV(), Min, BaseFOV, 0.f, 1.f);
    if(FOVPerc > 1.f) { FOVPerc = 1.f; }

    //Lerp using FOVPerc to get the rotation scale between *m_FOVRotationScale and 1.f
    float Output = (1.f - FOVPerc) * *m_FOVRotationScale + FOVPerc;// * 1.f

    return Output;
}

float CameraManager::GetClampedFOV(float InCurrentFOV, float Min, float Max)
{
    if(InCurrentFOV < Min)
    {
        FOVSpeed = 0.f;
        InCurrentFOV = Min;
    }
    if(InCurrentFOV > Max)
    {
        FOVSpeed = 0.f;
        InCurrentFOV = Max;
    }

    return InCurrentFOV;
}

float CameraManager::RemapPercentage(float CurrentPerc, float CurrentMin, float CurrentMax, float NewMin, float NewMax)
{
    return NewMin + (NewMax - NewMin) * ((CurrentPerc - CurrentMin) / (CurrentMax - CurrentMin));
}


// TESTS - REMOVE WHEN DONE //
void CameraManager::DebugRender(CanvasWrapper Canvas)
{
    if(!IsValidMode())
    {
        return;
    }

    //Get the camera's speed as a percentage per axis
    RT::Matrix3 MovementMatrix = GetCameraMatrix(*m_bUseLocalMovement, true);
    RT::Matrix3 RotationMatrix = GetCameraMatrix(*m_bUseLocalRotation, false);
    float ForwardSpeedPerc  = GetSpeedComponent(MovementMatrix.forward);
    float RightSpeedPerc    = GetSpeedComponent(MovementMatrix.right);
    float UpSpeedPerc       = GetSpeedComponent(MovementMatrix.up);
    float FOVScaleReduction = GetFOVScaleReduction();
    float PitchSpeedPerc    = GetAngularSpeedComponent(RotationMatrix.right);
    float YawSpeedPerc      = GetAngularSpeedComponent(RotationMatrix.up);
    float RollSpeedPerc     = GetAngularSpeedComponent(RotationMatrix.forward);
    
    //FOV junk
    float FOVMaxSpeed      = BaseFOVSpeed * *m_FOVSpeed;
    float FOVSpeedPerc     = FOVSpeed / FOVMaxSpeed;
    float FOVAccelSpeed    = BaseFOVAccel * *m_FOVAcceleration;
    float FOVImpulse       = FOVMaxSpeed * .0083f * FOVAccelSpeed;
    float FOVAcceleration  = Inputs->GetFOV() * GetReducedPerc(Inputs->GetFOV(), FOVSpeedPerc) * FOVImpulse;

    //Angular junk
    Vector RotationAxis = AngularVelocity.getNormalized();
    float RotationAmount = AngularVelocity.magnitude() * 0.008333f * (CONST_PI_F / 180.f);//magic number is roughly the same as Delta in playertick, and converting from deg to rad
    Quat NewRotation = AngleAxisRotation(RotationAmount, RotationAxis);
    
    //Create RenderStrings and fill it with some values
    std::vector<std::string> RenderStrings;
    RenderStrings.push_back("bUsingGamepad: " + std::to_string(Inputs->GetbUsingGamepad()));
    RenderStrings.push_back("Forward: "       + std::to_string(Inputs->GetForward()));
    RenderStrings.push_back("Right: "         + std::to_string(Inputs->GetRight()));
    RenderStrings.push_back("Up: "            + std::to_string(Inputs->GetUp()));
    RenderStrings.push_back("Pitch: "         + std::to_string(Inputs->GetPitch()));
    RenderStrings.push_back("Yaw: "           + std::to_string(Inputs->GetYaw()));
    RenderStrings.push_back("Roll: "          + std::to_string(Inputs->GetRoll()));
    RenderStrings.push_back("FOV: "           + std::to_string(Inputs->GetFOV()));
    RenderStrings.push_back("bRoll: "         + std::to_string(Inputs->GetbRoll()));
    RenderStrings.push_back("bFOV: "          + std::to_string(Inputs->GetbFOV()));
    RenderStrings.emplace_back("");
    RenderStrings.push_back("Total Velocity: "   + CBUtils::PrintFloat(VelocityWorld.magnitude(), 6));
    RenderStrings.push_back("Forward Velocity: " + CBUtils::PrintFloat(ForwardSpeedPerc, 4));
    RenderStrings.push_back("Right Velocity: "   + CBUtils::PrintFloat(RightSpeedPerc, 4));
    RenderStrings.push_back("Up Velocity: "      + CBUtils::PrintFloat(UpSpeedPerc, 4));
    RenderStrings.emplace_back("");
    RenderStrings.push_back("Total Angular Velocity: " + CBUtils::PrintVector(AngularVelocity, 6));
    RenderStrings.push_back("Pitch Velocity: "         + CBUtils::PrintFloat(PitchSpeedPerc, 4));
    RenderStrings.push_back("Yaw Velocity: "           + CBUtils::PrintFloat(YawSpeedPerc, 4));
    RenderStrings.push_back("Roll Velocity: "          + CBUtils::PrintFloat(RollSpeedPerc, 4));
    RenderStrings.emplace_back("");
    RenderStrings.push_back("Rotation Axis: "   + CBUtils::PrintVector(RotationAxis, 6));
    RenderStrings.push_back("Rotation Amount: " + CBUtils::PrintFloat(RotationAmount, 4));
    RenderStrings.push_back("New Rotation: "    + CBUtils::PrintQuat(NewRotation, 4));
    RenderStrings.emplace_back("");
    RenderStrings.push_back("FOV Speed: "        + CBUtils::PrintFloat(FOVSpeed, 6));
    RenderStrings.push_back("FOV Acceleration: " + CBUtils::PrintFloat(FOVAcceleration, 6));
    RenderStrings.push_back("FOV Scale Reduction: " + CBUtils::PrintFloat(FOVScaleReduction, 6));
    if(!GlobalGameWrapper->GetCamera().IsNull())
    {
        RenderStrings.push_back("FOV: " + CBUtils::PrintFloat(GlobalGameWrapper->GetCamera().GetFOV(), 4));
    }


    //Draw black box behind text
    Canvas.SetPosition(Vector2{50, 50});
    Canvas.SetColor(LinearColor{0, 0, 0, 200});
    Canvas.FillBox(Vector2{300, 50 + static_cast<int>(RenderStrings.size()) * 20});

    //Draw text
	Vector2 base = {75, 75};
    Canvas.SetColor(LinearColor{0, 255, 0, 255});
    for(const auto& Str : RenderStrings)
    {
        Canvas.SetPosition(base);
        Canvas.DrawString(Str);
        base.Y += 20;
    }
}
