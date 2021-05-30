#include "CameraManager.h"
#include "InputsManager.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/CBMatrix3.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "CameraConfigManager.h"
#include "UI/UIManager.h"

CameraManager::CameraManager(std::shared_ptr<UIManager> TheUI)
{
    UI = TheUI;
    Inputs = std::make_shared<InputsManager>(TheUI);

    //Register cvars
    UI->AddElement({bUseOverrides,        CVAR_ENABLE_CAM_OVERRIDE, "Enable Overrides",                   "Enables camera overriding features"                            });
    UI->AddElement({bLocalMomentum,       CVAR_CAM_LOCAL_MOMENTUM,  "Local Momentum Preservation",        "Preserves momentum in local space instead of world space"      });
    UI->AddElement({bUseLocalMovement,    CVAR_CAM_LOCAL_MOVEMENT,  "Local Movement",                     "Uses the local orientation of the camera for movement"         });
    UI->AddElement({bUseLocalRotation,    CVAR_CAM_LOCAL_ROTATION,  "Local Rotation",                     "Uses the local orientation of the camera for rotation"         });
    UI->AddElement({bHardFloors,          CVAR_CAM_HARD_FLOORS,     "Hard Floors",                        "Prevents the camera from going through the floor"              });
    UI->AddElement({FloorHeight,          CVAR_CAM_FLOOR_HEIGHT,    "Floor Height",                       "Lowest height the camera can go",                      -50, 50 });
    UI->AddElement({MovementSpeed,        CVAR_CAM_MOVEMENT_SPEED,  "Movement Speed",                     "Camera movement speed multiplier",                      0,  5  });
    UI->AddElement({MovementAccel,        CVAR_CAM_MOVEMENT_ACCEL,  "Movement Acceleration",              "Camera movement acceleration speed",                    0,  5  });
	UI->AddElement({RotationSpeedMouse,   CVAR_ROT_SPEED_MOUSE,     "Rotation Speed (Mouse)",             "Camera rotation speed (mouse)",                         0,  3  });
	UI->AddElement({RotationSpeedGamepad, CVAR_ROT_SPEED_GAMEPAD,   "Rotation Speed (Non-Mouse)",         "Camera rotation speed (non-mouse)",                     0,  3  });
    UI->AddElement({RotationAccelMouse,   CVAR_ROT_ACCEL_MOUSE,     "Rotation Acceleration (Mouse)",      "Camera rotation acceleration speed (mouse)",            0,  10 });
    UI->AddElement({RotationAccelGamepad, CVAR_ROT_ACCEL_GAMEPAD,   "Rotation Acceleration (Non-Mouse)",  "Camera rotation acceleration speed (controller)",       0,  10 });
    UI->AddElement({FOVRotationScale,     CVAR_FOV_ROTATION_SCALE,  "FOV Rotation Scale",                 "Multiplier for slowing camera rotation when zoomed in", 0,  1  });
    UI->AddElement({FOVMin,               CVAR_FOV_MIN,             "FOV Minimum",                        "The lowest the FOV will go",                            5,  170});
    UI->AddElement({FOVMax,               CVAR_FOV_MAX,             "FOV Maximum",                        "The highest the FOV will go",                           5,  170});
    UI->AddElement({FOVSpeed,             CVAR_FOV_SPEED,           "FOV Speed",                          "The speed at which FOV will change",                    0,  3  });
    UI->AddElement({FOVAcceleration,      CVAR_FOV_ACCELERATION,    "FOV Acceleration",                   "The easing of FOV speed change",                        0,  10 });
    UI->AddElement({FOVLimitEase,         CVAR_FOV_LIMIT_EASE,      "FOV Limit Ease",                     "Cutoff for easing effect into min/max FOV",             0,  .5f});

    //Reset the speeds to 0 when CameraManager has been enabled or disabled
    ON_CVAR_CHANGED(CVAR_ENABLE_CAM_OVERRIDE, CameraManager::ResetSpeeds);

    //Register notifiers
    MAKE_NOTIFIER(NOTIFIER_CAM_RESET, ResetSpeeds, "Resets camera acceleration and speed");

    //Register hooks
    GlobalGameWrapper->HookEvent("Function TAGame.PlayerInput_TA.PlayerInput", std::bind(&CameraManager::PlayerInputTick, this));

    //Create the config manager after all cvars have been created
    Configs = std::make_shared<CameraConfigManager>(TheUI);
}

void CameraManager::PlayerInputTick()
{
    //Get delta regardless of validity so there isn't suddenly a massive jump when it becomes valid again
    float Delta = GetDelta();

    //Determine if function should run. If it was previously running but now shouldn't, reset the speeds
    static bool bWasValid = true;
    if(!IsValidMode())
    {
        if(bWasValid)
        {
            bWasValid = false;
            ResetSpeeds();
        }

        return;
    }
    bWasValid = true;

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
        UpdatePosition(Delta, TheCamera);
        UpdateRotation(Delta, TheCamera);
        UpdateFOV(Delta, TheCamera);
    }
}

void CameraManager::UpdatePosition(float Delta, CameraWrapper TheCamera)
{
    //Update velocities
    Vector MovementInputs = GetMovementInputs();
    UpdateVelocityLocal(Delta, MovementInputs);
    UpdateVelocityWorld(Delta, MovementInputs);
    if((*bLocalMomentum && VelocityLocal.magnitude() < .00001f) || (!*bLocalMomentum && VelocityWorld.magnitude() < .00001f))
    {
        return;
    }

    //Update location based on chosen velocity type
    Vector NewLocation = TheCamera.GetLocation();
    if(*bLocalMomentum)
    {
        CBUtils::Matrix3 MovementMatrix = GetCameraMatrix(*bUseLocalMovement, true);
        NewLocation += MovementMatrix.Forward * VelocityLocal.X * Delta;
        NewLocation += MovementMatrix.Right   * VelocityLocal.Y * Delta;
        NewLocation += MovementMatrix.Up      * VelocityLocal.Z * Delta;
    }
    else
    {
        NewLocation += VelocityWorld * Delta;
    }

    //Clamp to above floor height
    if(*bHardFloors && NewLocation.Z < *FloorHeight)
    {
        NewLocation.Z = *FloorHeight;
    }

    //Apply the new location
    TheCamera.SetLocation(NewLocation);
}

void CameraManager::UpdateRotation(float Delta, CameraWrapper TheCamera)
{
    //Update angular velocity
    UpdateAngularVelocity(Delta, GetRotationInputs());
    if(AngularVelocity.magnitude() < .00001f)
    {
        return;
    }

    //Update rotation based on angular velocity
    constexpr float DegToRad = CONST_PI_F / 180.f;
    float PitchAmount = AngularVelocity.X * Delta * DegToRad;
    float YawAmount   = AngularVelocity.Y * Delta * DegToRad;
    float RollAmount  = AngularVelocity.Z * Delta * DegToRad;

    CBUtils::Matrix3 RotationMatrix = GetCameraMatrix(*bUseLocalRotation, false);
    Quat PitchRot = AngleAxisRotation(PitchAmount, RotationMatrix.Right);
    Quat YawRot   = AngleAxisRotation(YawAmount,   RotationMatrix.Up);
    Quat RollRot  = AngleAxisRotation(RollAmount,  RotationMatrix.Forward);

    //Apply the new rotations to the current rotation, then apply to the camera
    CBUtils::Matrix3 CurrentMatrix(TheCamera.GetRotation());
    CurrentMatrix.RotateWithQuat(PitchRot);
    CurrentMatrix.RotateWithQuat(YawRot);
    CurrentMatrix.RotateWithQuat(RollRot);
    TheCamera.SetRotation(CurrentMatrix.ToRotator());
}

void CameraManager::UpdateFOV(float Delta, CameraWrapper TheCamera)
{
    UpdateFOVSpeed(Delta);

    if(abs(FOVCurrentSpeed) < .00001f)
    {
        return;
    }

    float Min = *FOVMin < *FOVMax ? *FOVMin : *FOVMax;
    float Max = *FOVMin < *FOVMax ? *FOVMax : *FOVMin;

    float CurrentFOV = TheCamera.GetFOV();
    float FOVPerc = RemapPercentage(CurrentFOV, Min, Max, 0.f, 1.f);
    bool bInLowerLimitRange = FOVPerc <= *FOVLimitEase;
    bool bInUpperLimitRange = FOVPerc >= 1.f - *FOVLimitEase;
    float ReductionStrength = 1.f;
    if(bInLowerLimitRange || bInUpperLimitRange)
    {
        //FOV is approaching limits. If FOVSpeed is continuing to head in that direction, reduce its strength
        //Since this is used as a multiplier, 1 is no reduction, 0 is full reduction
        if(bInLowerLimitRange && FOVCurrentSpeed < 0.f)
        {
            ReductionStrength = RemapPercentage(FOVPerc, 0.f, *FOVLimitEase, 0.f, 1.f);
        }
        else if(bInUpperLimitRange && FOVCurrentSpeed > 0.f)
        {
            ReductionStrength = 1.f - RemapPercentage(FOVPerc, 1.f - *FOVLimitEase, 1.f, 0.f, 1.f);
        }
    }

    float NewFOV = TheCamera.GetFOV() + FOVCurrentSpeed * Delta * ReductionStrength;
    NewFOV = GetClampedFOV(NewFOV, Min, Max);
    TheCamera.SetFOV(NewFOV);
}


// SPEED CALCULATIONS //
void CameraManager::ResetSpeeds()
{
    if(IsValidMode())
    {
        Inputs->ResetInputs(false);
    }

    VelocityWorld   = {0, 0, 0};
    VelocityLocal   = {0, 0, 0};
    AngularVelocity = {0, 0, 0};
    FOVCurrentSpeed = 0.f;
}

void CameraManager::UpdateVelocityLocal(float Delta, Vector MovementInputs)
{
    float MaxSpeed   = BaseMovementSpeed * *MovementSpeed;
    float AccelSpeed = BaseMovementAccel * *MovementAccel;
    VelocityLocal.X += UpdateFloatSpeed(Delta, VelocityLocal.X, MovementInputs.X, MaxSpeed, AccelSpeed);
    VelocityLocal.Y += UpdateFloatSpeed(Delta, VelocityLocal.Y, MovementInputs.Y, MaxSpeed, AccelSpeed);
    VelocityLocal.Z += UpdateFloatSpeed(Delta, VelocityLocal.Z, MovementInputs.Z, MaxSpeed, AccelSpeed);
}

void CameraManager::UpdateVelocityWorld(float Delta, Vector MovementInputs)
{
    float MaxSpeed   = BaseMovementSpeed * *MovementSpeed;
    float AccelSpeed = BaseMovementAccel * *MovementAccel;

    Vector CurrentSpeed;
    CBUtils::Matrix3 MovementMatrix = GetCameraMatrix(*bUseLocalMovement, true);
    CurrentSpeed.X = Vector::dot(VelocityWorld, MovementMatrix.Forward);
    CurrentSpeed.Y = Vector::dot(VelocityWorld, MovementMatrix.Right);
    CurrentSpeed.Z = Vector::dot(VelocityWorld, MovementMatrix.Up);

    Vector ForwardForce = UpdateFloatSpeed(Delta, CurrentSpeed.X, MovementInputs.X, MaxSpeed, AccelSpeed) * MovementMatrix.Forward;
    Vector RightForce   = UpdateFloatSpeed(Delta, CurrentSpeed.Y, MovementInputs.Y, MaxSpeed, AccelSpeed) * MovementMatrix.Right;
    Vector UpForce      = UpdateFloatSpeed(Delta, CurrentSpeed.Z, MovementInputs.Z, MaxSpeed, AccelSpeed) * MovementMatrix.Up;

    VelocityWorld += ForwardForce;
    VelocityWorld += RightForce;
    VelocityWorld += UpForce;
}

void CameraManager::UpdateAngularVelocity(float Delta, Vector RotationInputs)
{
    float MaxSpeedGamepad   = BaseRotationSpeedGamepad * *RotationSpeedGamepad;
    float AccelSpeedGamepad = BaseRotationAccelGamepad * *RotationAccelGamepad;
    float MaxSpeedMouse     = BaseRotationSpeedMouse   * *RotationSpeedMouse;
    float AccelSpeedMouse   = BaseRotationAccelMouse   * *RotationAccelMouse;
    
    if(Inputs->GetbUsingGamepad())
    {
        AngularVelocity.X += UpdateFloatSpeed(Delta, AngularVelocity.X, RotationInputs.X, MaxSpeedGamepad * GetFOVScaleReduction(), AccelSpeedGamepad);
        AngularVelocity.Y += UpdateFloatSpeed(Delta, AngularVelocity.Y, RotationInputs.Y, MaxSpeedGamepad * GetFOVScaleReduction(), AccelSpeedGamepad);
    }
    else
    {
        AngularVelocity.X += UpdateFloatSpeed(Delta, AngularVelocity.X, RotationInputs.X, MaxSpeedMouse * GetFOVScaleReduction(), AccelSpeedMouse);
        AngularVelocity.Y += UpdateFloatSpeed(Delta, AngularVelocity.Y, RotationInputs.Y, MaxSpeedMouse * GetFOVScaleReduction(), AccelSpeedMouse);
    }

    AngularVelocity.Z += UpdateFloatSpeed(Delta, AngularVelocity.Z, RotationInputs.Z, MaxSpeedGamepad, AccelSpeedGamepad);
}

void CameraManager::UpdateFOVSpeed(float Delta)
{
    float Input = Inputs->GetFOV() * -1.f;
    float MaxSpeed   = BaseFOVSpeed * *FOVSpeed;
    float AccelSpeed = BaseFOVAccel * *FOVAcceleration;
    FOVCurrentSpeed += UpdateFloatSpeed(Delta, FOVCurrentSpeed, Input, MaxSpeed, AccelSpeed);
}

float CameraManager::UpdateFloatSpeed(float Delta, float CurrentSpeed, float Input, float MaxSpeed, float AccelSpeed)
{
    //Calculate some variables used throughout the function
    float ImpulseStrength = MaxSpeed * Delta * AccelSpeed;
    float SpeedPerc = CurrentSpeed / MaxSpeed;

    //Calculate acceleration
    float Acceleration = Input * GetReducedPerc(Input, SpeedPerc);

    //Calculate brake
    float Brake = GetBrakeForce(Input, SpeedPerc);

    //Apply the impulses
    return (Acceleration - Brake) * ImpulseStrength;
}


// INPUTS //
Vector CameraManager::GetMovementInputs()
{
    Vector Output;

    Output.X = Inputs->GetForward();
    Output.Y = Inputs->GetRight();
    Output.Z = Inputs->GetUp();

    return Output;
}

Vector CameraManager::GetRotationInputs()
{
    Vector Output;

    Output.X = Inputs->GetPitch() * -1.f;
    Output.Y = Inputs->GetYaw();
    Output.Z = Inputs->GetRoll()  * -1.f;

    return Output;
}


// UTILITY //
bool CameraManager::IsValidMode()
{
    if(!*bUseOverrides)
    {
        return false;
    }

    if(!GlobalGameWrapper->GetLocalCar().IsNull())
    {
        return false;
    }

    if(GlobalGameWrapper->GetCurrentGameState().IsNull())
    {
        return false;
    }

    PlayerControllerWrapper PCW = GlobalGameWrapper->GetPlayerController();
    if(!PCW.IsNull())
    {
        PriWrapper PRI = PCW.GetPRI();
        if(!PRI.IsNull())
        {
            int TeamNum = PRI.GetTeamNum2();
            if(TeamNum == 0 || TeamNum == 1)
            {
                return false;
            }
        }
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

CBUtils::Matrix3 CameraManager::GetCameraMatrix(bool bFullyLocal, bool bLocationMatrix)
{
    CameraWrapper TheCamera = GlobalGameWrapper->GetCamera();
    if(TheCamera.IsNull())
    {
        return CBUtils::Matrix3();
    }

    //Return the matrix constructed from the camera's current orientation
    if(bFullyLocal)
    {
        return CBUtils::Matrix3(TheCamera.GetRotation());
    }

    //Approximate the game's camera matrices. Local and rotation act differently
    CBUtils::Matrix3 Output(TheCamera.GetRotation());
    Output.Up = Vector(0, 0, 1);
    if(bLocationMatrix)
    {
        Output.Forward.Z = 0.f; Output.Forward.normalize();
        Output.Right.Z   = 0.f; Output.Right.normalize();
    }
    else
    {
        Vector NewRight = Vector::cross(Output.Up, Output.Forward).getNormalized();
        if(Vector::dot(NewRight, Output.Right) < 0.f)
        {
            NewRight *= -1.f;
        }
        Output.Right = NewRight;
    }

    return Output;
}

float CameraManager::GetInvertedPerc(float InPerc)
{
    return (InPerc >= 0.f) ? (1.f - InPerc) : (1.f + InPerc);
}

float CameraManager::GetWeightedPerc(float InPerc)
{
    //Some ease functions don't like negative values
    //Preserve the negative status for later and use absolute value
    float NegativeMult = InPerc < 0.f ? -1.f : 1.f;
    InPerc = abs(InPerc);

    //Ease-out
    float EasedPerc = sinf((InPerc * CONST_PI_F) / 2.f);

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
    float Min = *FOVMin < *FOVMax ? *FOVMin : *FOVMax;
    float FOVPerc = RemapPercentage(TheCamera.GetFOV(), Min, BaseFOV, 0.f, 1.f);
    if(FOVPerc > 1.f) { FOVPerc = 1.f; }

    //Lerp using FOVPerc to get the rotation scale between *m_FOVRotationScale and 1.f
    float Output = (1.f - FOVPerc) * *FOVRotationScale + FOVPerc;// * 1.f

    return Output;
}

float CameraManager::GetClampedFOV(float InCurrentFOV, float Min, float Max)
{
    if(InCurrentFOV < Min)
    {
        FOVCurrentSpeed = 0.f;
        InCurrentFOV = Min;
    }
    if(InCurrentFOV > Max)
    {
        FOVCurrentSpeed = 0.f;
        InCurrentFOV = Max;
    }

    return InCurrentFOV;
}

float CameraManager::RemapPercentage(float CurrentPerc, float CurrentMin, float CurrentMax, float NewMin, float NewMax)
{
    return NewMin + (NewMax - NewMin) * ((CurrentPerc - CurrentMin) / (CurrentMax - CurrentMin));
}
