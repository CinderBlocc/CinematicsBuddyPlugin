#include "CameraManager.h"
#include "InputsManager.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "RenderingTools/RenderingTools.h"
#include "CameraConfigManager.h"
#include "UI/UIManager.h"

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
	UI->AddElement({m_RotationSpeedMouse,   CVAR_ROT_SPEED_MOUSE,     "Rotation Speed (mouse)",             "Camera rotation speed (mouse)",                         0,  3  });
	UI->AddElement({m_RotationSpeedGamepad, CVAR_ROT_SPEED_GAMEPAD,   "Rotation Speed (non-mouse)",         "Camera rotation speed (non-mouse)",                     0,  3  });
    UI->AddElement({m_RotationAccelMouse,   CVAR_ROT_ACCEL_MOUSE,     "Rotation Acceleration (Mouse)",      "Camera rotation acceleration speed (mouse)",            0,  10 });
    UI->AddElement({m_RotationAccelGamepad, CVAR_ROT_ACCEL_GAMEPAD,   "Rotation Acceleration (Controller)", "Camera rotation acceleration speed (controller)",       0,  10 });
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
    if((*m_bLocalMomentum && VelocityLocal.magnitude() < .00001f) || (!*m_bLocalMomentum && VelocityWorld.magnitude() < .00001f))
    {
        return;
    }

    //Update location based on chosen velocity type
    Vector NewLocation = TheCamera.GetLocation();
    if(*m_bLocalMomentum)
    {
        RT::Matrix3 MovementMatrix = GetCameraMatrix(*m_bUseLocalMovement, true);
        NewLocation += MovementMatrix.forward * VelocityLocal.X * Delta;
        NewLocation += MovementMatrix.right   * VelocityLocal.Y * Delta;
        NewLocation += MovementMatrix.up      * VelocityLocal.Z * Delta;
    }
    else
    {
        NewLocation += VelocityWorld * Delta;
    }

    //Clamp to above floor height
    if(*m_bHardFloors && NewLocation.Z < *m_FloorHeight)
    {
        NewLocation.Z = *m_FloorHeight;
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

    RT::Matrix3 RotationMatrix = GetCameraMatrix(*m_bUseLocalRotation, false);
    Quat PitchRot = AngleAxisRotation(PitchAmount, RotationMatrix.right);
    Quat YawRot   = AngleAxisRotation(YawAmount,   RotationMatrix.up);
    Quat RollRot  = AngleAxisRotation(RollAmount,  RotationMatrix.forward);

    //Apply the new rotations to the current rotation, then apply to the camera
    RT::Matrix3 CurrentMatrix(TheCamera.GetRotation());
    CurrentMatrix.RotateWithQuat(PitchRot);
    CurrentMatrix.RotateWithQuat(YawRot);
    CurrentMatrix.RotateWithQuat(RollRot);
    TheCamera.SetRotation(CurrentMatrix.ToRotator());
}

void CameraManager::UpdateFOV(float Delta, CameraWrapper TheCamera)
{
    UpdateFOVSpeed(Delta);

    if(abs(FOVSpeed) < .00001f)
    {
        return;
    }

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


// SPEED CALCULATIONS //
void CameraManager::UpdateVelocityLocal(float Delta, Vector MovementInputs)
{
    float MaxSpeed   = BaseMovementSpeed * *m_MovementSpeed;
    float AccelSpeed = BaseMovementAccel * *m_MovementAccel;
    VelocityLocal.X += UpdateFloatSpeed(Delta, VelocityLocal.X, MovementInputs.X, MaxSpeed, AccelSpeed);
    VelocityLocal.Y += UpdateFloatSpeed(Delta, VelocityLocal.Y, MovementInputs.Y, MaxSpeed, AccelSpeed);
    VelocityLocal.Z += UpdateFloatSpeed(Delta, VelocityLocal.Z, MovementInputs.Z, MaxSpeed, AccelSpeed);
}

void CameraManager::UpdateVelocityWorld(float Delta, Vector MovementInputs)
{
    float MaxSpeed   = BaseMovementSpeed * *m_MovementSpeed;
    float AccelSpeed = BaseMovementAccel * *m_MovementAccel;

    Vector CurrentSpeed;
    RT::Matrix3 MovementMatrix = GetCameraMatrix(*m_bUseLocalMovement, true);
    CurrentSpeed.X = Vector::dot(VelocityWorld, MovementMatrix.forward);
    CurrentSpeed.Y = Vector::dot(VelocityWorld, MovementMatrix.right);
    CurrentSpeed.Z = Vector::dot(VelocityWorld, MovementMatrix.up);

    Vector ForwardForce = UpdateFloatSpeed(Delta, CurrentSpeed.X, MovementInputs.X, MaxSpeed, AccelSpeed) * MovementMatrix.forward;
    Vector RightForce   = UpdateFloatSpeed(Delta, CurrentSpeed.Y, MovementInputs.Y, MaxSpeed, AccelSpeed) * MovementMatrix.right;
    Vector UpForce      = UpdateFloatSpeed(Delta, CurrentSpeed.Z, MovementInputs.Z, MaxSpeed, AccelSpeed) * MovementMatrix.up;

    VelocityWorld += ForwardForce;
    VelocityWorld += RightForce;
    VelocityWorld += UpForce;
}

void CameraManager::UpdateAngularVelocity(float Delta, Vector RotationInputs)
{
    float MaxSpeedGamepad   = BaseRotationSpeedGamepad * *m_RotationSpeedGamepad;
    float AccelSpeedGamepad = BaseRotationAccelGamepad * *m_RotationAccelGamepad;
    float MaxSpeedMouse     = BaseRotationSpeedMouse * *m_RotationSpeedMouse;
    float AccelSpeedMouse   = BaseRotationAccelMouse * *m_RotationAccelMouse;
    
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
    float MaxSpeed   = BaseFOVSpeed * *m_FOVSpeed;
    float AccelSpeed = BaseFOVAccel * *m_FOVAcceleration;
    FOVSpeed += UpdateFloatSpeed(Delta, FOVSpeed, Input, MaxSpeed, AccelSpeed);
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
    if(!*m_bUseOverrides)
    {
        return false;
    }

    if(!GlobalGameWrapper->GetLocalCar().IsNull())
    {
        return false;
    }

    PlayerControllerWrapper PCW = GlobalGameWrapper->GetPlayerController();
    if(!PCW.IsNull() && !GlobalGameWrapper->GetCurrentGameState().IsNull())
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

    //Approximate the game's camera matrices. Local and rotation act differently
    RT::Matrix3 Output(TheCamera.GetRotation());
    Output.up = Vector(0, 0, 1);
    if(bLocationMatrix)
    {
        Output.forward.Z = 0.f; Output.forward.normalize();
        Output.right.Z   = 0.f; Output.right.normalize();
    }
    else
    {
        Vector NewRight = Vector::cross(Output.up, Output.forward).getNormalized();
        if(Vector::dot(NewRight, Output.right) < 0.f)
        {
            NewRight *= -1.f;
        }
        Output.right = NewRight;
    }

    return Output;
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
