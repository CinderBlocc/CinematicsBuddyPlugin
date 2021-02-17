#include "CameraManager.h"
#include "InputsManager.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "BMGraphs/BMGraphs/BMGraphs.h"

/*

    #TODO:
        - Separate the camera matrix into two separate ones?
            - One for local location and one for local rotation
            - Sometimes people might want the old "world space" movement, but keep the local rotation

        - Option to read configs and apply their values to the camera

*/

CameraManager::CameraManager()
{
    Inputs = std::make_shared<InputsManager>();

    //Register cvars
    MAKE_CVAR_BIND_TO_STRING(m_bUseOverrides,        CVAR_ENABLE_CAM_OVERRIDE, "Enables camera overriding features",       true);
    MAKE_CVAR_BIND_TO_STRING(m_bUseLocalMatrix,      CVAR_CAM_LOCAL_MATRIX,    "Uses the local orientation of the camera", true);
	MAKE_CVAR_BIND_TO_STRING(m_MovementSpeed,        CVAR_CAM_MOVEMENT_SPEED,  "Camera movement speed multiplier",                        true, true, 0, true, 5);
    MAKE_CVAR_BIND_TO_STRING(m_MovementAccel,        CVAR_CAM_MOVEMENT_ACCEL,  "Camera movement acceleration speed",                      true, true, 0, true, 5);
	MAKE_CVAR_BIND_TO_STRING(m_RotationSpeed,        CVAR_ROT_SPEED,           "Camera rotation speed multiplier (doesn't affect mouse)", true, true, 0, true, 3);
    MAKE_CVAR_BIND_TO_STRING(m_RotationAccelMouse,   CVAR_ROT_ACCEL_MOUSE,     "Camera rotation acceleration speed (mouse)",              true, true, 0, true, 5);
    MAKE_CVAR_BIND_TO_STRING(m_RotationAccelGamepad, CVAR_ROT_ACCEL_GAMEPAD,   "Camera rotation acceleration speed (controller)",         true, true, 0, true, 5);
    MAKE_CVAR_BIND_TO_STRING(m_MouseSensitivity,     CVAR_MOUSE_SENSITIVITY,   "Camera rotation speed when using mouse",                  true, true, 0, true, 25);
    MAKE_CVAR_BIND_TO_STRING(m_GamepadSensitivity,   CVAR_GAMEPAD_SENSITIVITY, "Camera rotation speed when using gamepad",                true, true, 0, true, 50);
    MAKE_CVAR_BIND_TO_STRING(m_FOVRotationScale,     CVAR_FOV_ROTATION_SCALE,  "Multiplier for slowing camera rotation when zoomed in",   true, true, 0, true, 2);
    MAKE_CVAR_BIND_STRING(   m_RollBinding,          CVAR_ROLL_BINDING,        "Button bound to bRoll modifier to change camera yaw input to roll", true);
    ON_CVAR_CHANGED(CVAR_ROLL_BINDING, CameraManager, CacheRollBinding);
    CacheRollBinding();

    //Register hooks
    GlobalGameWrapper->HookEvent("Function TAGame.PlayerInput_TA.PlayerInput", std::bind(&CameraManager::PlayerInputTick, this));

    // TESTS - REMOVE WHEN DONE //
    ON_CVAR_CHANGED(CVAR_ENABLE_CAM_OVERRIDE, CameraManager, OnUseOverridesChanged);
    Graphs = std::make_shared<BMGraphs>(GlobalCvarManager, GlobalGameWrapper);
}

void CameraManager::OnUseOverridesChanged()
{
    // TESTS - REMOVE WHEN DONE //
    Graphs->EndRender();
    if(*m_bUseOverrides)
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

void CameraManager::CacheRollBinding()
{
    RollBindingIndex = GlobalGameWrapper->GetFNameIndexByString(*m_RollBinding);
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
    bRoll = GlobalGameWrapper->IsKeyPressed(RollBindingIndex);
	Inputs->PlayerInputTick(Delta, bRoll);

    //Apply inputs to camera
    UpdateCameraTransformation(Delta);
}

void CameraManager::UpdateCameraTransformation(float Delta)
{
    CameraWrapper TheCamera = GlobalGameWrapper->GetCamera();
    if(!TheCamera.IsNull())
    {
        UpdateCameraMatrix(TheCamera);
        UpdateVelocity(Delta);
        UpdateAngularVelocity(Delta);
        UpdatePosition(Delta, TheCamera);
        UpdateRotation(Delta, TheCamera);
    }
}

void CameraManager::UpdateCameraMatrix(CameraWrapper TheCamera)
{
    if(*m_bUseLocalMatrix)
    {
        //Use fully local matrix
        Quat CameraQuat = RotatorToQuat(TheCamera.GetRotation());
        Forward = RotateVectorWithQuat(Vector(1, 0, 0), CameraQuat).getNormalized();
        Right   = RotateVectorWithQuat(Vector(0, 1, 0), CameraQuat).getNormalized();
        Up      = RotateVectorWithQuat(Vector(0, 0, 1), CameraQuat).getNormalized();
    }
    else
    {
        //Approximate the game's camera matrix
        Quat CameraQuat = RotatorToQuat(TheCamera.GetRotation());
        Forward = RotateVectorWithQuat(Vector(1, 0, 0), CameraQuat); Forward.Z = 0.f; Forward.normalize();
        Right   = RotateVectorWithQuat(Vector(0, 1, 0), CameraQuat); Right.Z   = 0.f; Right.normalize();
        Up      = Vector(0, 0, 1);
    }
}

void CameraManager::UpdateVelocity(float Delta)
{
    //Calculate some variables used throughout the function
    float MaxSpeed = BaseMovementSpeed * *m_MovementSpeed;
    float AccelSpeed = BaseMovementAccel * *m_MovementAccel;
    float ImpulseStrength = MaxSpeed * Delta * AccelSpeed;

    //Get the camera's speed as a percentage per axis
    float ForwardSpeedPerc = GetSpeedComponent(Forward);
    float RightSpeedPerc   = GetSpeedComponent(Right);
    float UpSpeedPerc      = GetSpeedComponent(Up);

    //Get input percentages
    float ForwardInputPerc = Inputs->GetForward();
    float RightInputPerc   = Inputs->GetRight();
    float UpInputPerc      = Inputs->GetUp();

    //Convert those inputs into vector inputs
    Vector ForwardInputVec = Forward * ForwardInputPerc;
    Vector RightInputVec   = Right   * RightInputPerc;
    Vector UpInputVec      = Up      * UpInputPerc;

    //Calculate acceleration force per axis
    Vector ForwardAcceleration = ForwardInputVec * GetReducedPerc(ForwardInputPerc, ForwardSpeedPerc);
    Vector RightAcceleration   = RightInputVec   * GetReducedPerc(RightInputPerc, RightSpeedPerc);
    Vector UpAcceleration      = UpInputVec      * GetReducedPerc(UpInputPerc, UpSpeedPerc);
    
    //Calculate impulse created by inputs
    Vector AccelerationDirection = ForwardAcceleration + RightAcceleration + UpAcceleration;
    Vector Acceleration = AccelerationDirection * ImpulseStrength;

    //Calculate automatic braking force (only apply brakes if no input on that axis)
    Vector ForwardBrake = Forward * GetBrakeForce(ForwardInputPerc, ForwardSpeedPerc);
    Vector RightBrake   = Right   * GetBrakeForce(RightInputPerc, RightSpeedPerc);
    Vector UpBrake      = Up      * GetBrakeForce(UpInputPerc, UpSpeedPerc);
    
    //Calculate impulse created by braking
    Vector BrakeDirection = ForwardBrake + RightBrake + UpBrake;
    Vector Brake = BrakeDirection * ImpulseStrength;

    //Apply the impulses
    Velocity += Acceleration - Brake;


    // TESTS - REMOVE WHEN DONE //
    static int EveryXNumber = 0;
    if(EveryXNumber == 3)
    {
        std::vector<LineGraphDataSingle> LineData;
        LineData.push_back({ "Forward Velocity",  ForwardSpeedPerc });
        LineData.push_back({ "Right Velocity",    RightSpeedPerc   });
        LineData.push_back({ "Up Velocity",       UpSpeedPerc      });
        LineData.push_back({ "Forward Input",     ForwardInputPerc });
        LineData.push_back({ "Right Input",       RightInputPerc   });
        LineData.push_back({ "Up Input",          UpInputPerc      });
        Graphs->InputData(LineData);
        EveryXNumber = 0;
    }
    ++EveryXNumber;
}

void CameraManager::UpdateAngularVelocity(float Delta)
{
    //#TODO: Take FOVRotationScale into account here, along with camera FOV

    //RotationSpeed should only be taken into account for Pitch and Yaw if bUsingGamepad is true
    //Roll is set by both keyboard and controller as a rate, along with Pitch and Yaw on controller
    //Pitch and Yaw are set by mouse via movement deltas which give large numbers, so speed should not be used

    float MaxSpeed = BaseRotationSpeed * *m_RotationSpeed;
}

void CameraManager::UpdatePosition(float Delta, CameraWrapper TheCamera)
{
    Vector NewLocation = TheCamera.GetLocation() + Velocity * Delta;
    TheCamera.SetLocation(NewLocation);
}

void CameraManager::UpdateRotation(float Delta, CameraWrapper TheCamera)
{
    
}


// UTILITY //
bool CameraManager::IsValidMode()
{
    //#TODO: Check if camera is spectator or something? Check only if they're in replay?

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

float CameraManager::GetSpeedComponent(Vector Direction)
{
    float MaxSpeed = BaseMovementSpeed * *m_MovementSpeed;
    return Vector::dot(Velocity, Direction) / MaxSpeed;
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

float CameraManager::RemapPercentage(float CurrentPerc, float CurrentMin, float CurrentMax, float NewMin, float NewMax)
{
    return NewMin + (NewMax - NewMin) * ((CurrentPerc - CurrentMin) / (CurrentMax - CurrentMin));
}


// TESTS - REMOVE WHEN DONE //
void CameraManager::StartInputsTest()
{
    Velocity = {0,0,0};
    CameraWrapper GameCam = GlobalGameWrapper->GetCamera();
    if(!GameCam.IsNull())
    {
        GameCam.SetLocation(Vector{0,0,100});
        GameCam.SetRotation(Rotator{0,0,0});
    }

    Inputs->RunTest();
}

void CameraManager::DebugRender(CanvasWrapper Canvas)
{
    if(!IsValidMode())
    {
        return;
    }

    //Get the camera's speed as a percentage per axis
    float ForwardSpeedPerc = GetSpeedComponent(Forward);
    float RightSpeedPerc   = GetSpeedComponent(Right);
    float UpSpeedPerc      = GetSpeedComponent(Up);
    
    //Create RenderStrings and fill it with some values
    std::vector<std::string> RenderStrings;
    RenderStrings.push_back("bRoll: "              + std::to_string(bRoll));
    //RenderStrings.push_back("MovementSpeed: "      + std::to_string(MovementSpeed));
    //RenderStrings.push_back("MovementAccel: "      + std::to_string(MovementAccel));
    //RenderStrings.push_back("RotationAccel: "      + std::to_string(RotationAccelMouse));
    //RenderStrings.push_back("MouseSensitivity: "   + std::to_string(MouseSensitivity));
    //RenderStrings.push_back("GamepadSensitivity: " + std::to_string(GamepadSensitivity));
    //RenderStrings.push_back("FOVRotationScale: "   + std::to_string(FOVRotationScale));

    //Get values from InputsManager
    Inputs->DebugRender(Canvas, RenderStrings);

    RenderStrings.emplace_back("");
    RenderStrings.push_back("Total Velocity: " + CBUtils::PrintFloat(Velocity.magnitude(), 6));
    RenderStrings.push_back("Forward Velocity: " + CBUtils::PrintFloat(ForwardSpeedPerc, 4));
    RenderStrings.push_back("Right Velocity: " + CBUtils::PrintFloat(RightSpeedPerc, 4));
    RenderStrings.push_back("Up Velocity: " + CBUtils::PrintFloat(UpSpeedPerc, 4));
    RenderStrings.push_back("AngularVelocity: " + CBUtils::PrintVector(AngularVelocity, 6));


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


    //Graphs->Render(Canvas);
}
