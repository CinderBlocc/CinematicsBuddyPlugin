#include "CameraManager.h"
#include "InputsManager.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "RenderingTools/RenderingTools.h"
#include "UI/UIManager.h"

// TESTS - REMOVE WHEN DONE //
#include "BMGraphs/BMGraphs/BMGraphs.h"

/*

    #TODO:
        - Why does rotation snap back to 0,0,0 sometimes?

        - When using non-local rotation and rolled, pitching will eventually reset roll back to 0
            - Maybe forward.Z shouldnt be set to 0, only right.Z should be?

        - Add mouse input

        - Option to read configs and apply their values to the camera
            - Make a custom parser? Not sure if the exec config command can user relative pathing
                - It would just be cleaner to go directly to the CB config folder anyway
                - Make a notifier like "CBExecConfig" that would take the config name as a parameter
                    - With a custom command like that, you could have subconfigs (configs that run a config, then change a couple settings after the first one runs)
                    - Should be able to use relative pathing starting from the main CB configs folder. Subfolders for certain projects or cine types would be nice

        - Option to preserve momentum in world space?
            - I think it already does that. To do local you might need to store each velocity component individually

        - FOV acceleration. Attach to left bumper?
            - Instead, for both pitch and FOV, add a dropdown menu letting the user choose which input should be swapped
                - Remove the pitch/roll checkbox once you add this
                - This will be much easier to add with templatized cvar creation. You could feed the settings into the struct and have them auto generate in UI

        - Take FOV into account for pitch and yaw rotation speed. The more zoomed in, the less speed

        - The goal with this is to entirely replace Spectator Controls' smoothing feature
            - Maybe even Camera Lock too? Might be better to keep that separate

*/

CameraManager::CameraManager(std::shared_ptr<UIManager> TheUI)
{
    Inputs = std::make_shared<InputsManager>(TheUI);
    UI = TheUI;

    //Register cvars
    UI->AddElement({m_bUseOverrides,        CVAR_ENABLE_CAM_OVERRIDE, "Enable Overrides",                   "Enables camera overriding features"                            });
    UI->AddElement({m_bUseLocalMovement,    CVAR_CAM_LOCAL_MOVEMENT,  "Local movement",                     "Uses the local orientation of the camera for movement"         });
    UI->AddElement({m_bUseLocalRotation,    CVAR_CAM_LOCAL_ROTATION,  "Local rotation",                     "Uses the local orientation of the camera for rotation"         });
    UI->AddElement({m_bHardFloors,          CVAR_CAM_HARD_FLOORS,     "Hard floors",                        "Prevents the camera from going through the floor"              });
    UI->AddElement({m_FloorHeight,          CVAR_CAM_FLOOR_HEIGHT,    "Floor Height",                       "Lowest height the camera can go",                      -50, 50 });
    UI->AddElement({m_MovementSpeed,        CVAR_CAM_MOVEMENT_SPEED,  "Movement Speed",                     "Camera movement speed multiplier",                      0,  5  });
    UI->AddElement({m_MovementAccel,        CVAR_CAM_MOVEMENT_ACCEL,  "Movement Acceleration",              "Camera movement acceleration speed",                    0,  5  });
	UI->AddElement({m_RotationSpeed,        CVAR_ROT_SPEED,           "Rotation Speed (non-mouse)",         "Camera rotation speed (non-mouse)",                     0,  3  });
    UI->AddElement({m_RotationAccelMouse,   CVAR_ROT_ACCEL_MOUSE,     "Rotation Acceleration (Mouse)",      "Camera rotation acceleration speed (mouse)",            0,  5  });
    UI->AddElement({m_RotationAccelGamepad, CVAR_ROT_ACCEL_GAMEPAD,   "Rotation Acceleration (Controller)", "Camera rotation acceleration speed (controller)",       0,  5  });
    UI->AddElement({m_MouseSensitivity,     CVAR_MOUSE_SENSITIVITY,   "Mouse Sensitivity",                  "Camera rotation speed when using mouse",                0,  25 });
    UI->AddElement({m_GamepadSensitivity,   CVAR_GAMEPAD_SENSITIVITY, "Gamepad Sensitivity",                "Camera rotation speed when using gamepad",              0,  50 });
    UI->AddElement({m_FOVRotationScale,     CVAR_FOV_ROTATION_SCALE,  "FOV Rotation Scale",                 "Multiplier for slowing camera rotation when zoomed in", 0,  2  });
    UI->AddElement({m_RollBinding,          CVAR_ROLL_BINDING,        "Toggle roll binding",                "Modifier to swap an input axis with roll"                      });
  //UI->AddElement({m_FOVBinding,           CVAR_FOV_BINDING,         "Toggle FOV binding",                 "Modifier to swap an input axis with FOV"                       });
    
    //Add options to dropdown menus
    SetBindingOptions();
    SetInputSwapOptions();

    //Bind addOnValueChanged functions to the input bindings
    ON_CVAR_CHANGED(CVAR_ROLL_BINDING, CameraManager::CacheRollBinding);
  //ON_CVAR_CHANGED(CVAR_FOV_BINDING, CameraManager::CacheFOVBinding);
    CacheRollBinding();
    CacheFOVBinding();

    //Register hooks
    GlobalGameWrapper->HookEvent("Function TAGame.PlayerInput_TA.PlayerInput", std::bind(&CameraManager::PlayerInputTick, this));

    // TESTS - REMOVE WHEN DONE //
    ON_CVAR_CHANGED(CVAR_ENABLE_CAM_OVERRIDE, CameraManager::OnUseOverridesChanged);
    Graphs = std::make_shared<BMGraphs>(GlobalCvarManager, GlobalGameWrapper);
}

void CameraManager::SetBindingOptions()
{
    UIElement::DropdownOptionsType BindingsList;
    BindingsList.emplace_back(NO_SELECTION, NO_SELECTION);
    BindingsList.emplace_back("Left thumbstick press", "XboxTypeS_LeftThumbStick");
    BindingsList.emplace_back("Right thumbstick press", "XboxTypeS_RightThumbStick");
    BindingsList.emplace_back("DPad up", "XboxTypeS_DPad_Up");
    BindingsList.emplace_back("DPad left", "XboxTypeS_DPad_Left");
    BindingsList.emplace_back("DPad right", "XboxTypeS_DPad_Right");
    BindingsList.emplace_back("DPad down", "XboxTypeS_DPad_Down");
    BindingsList.emplace_back("Back button", "XboxTypeS_Back");
    BindingsList.emplace_back("Start button", "XboxTypeS_Start");
    BindingsList.emplace_back("Xbox Y - PS4 Triangle", "XboxTypeS_Y");
    BindingsList.emplace_back("Xbox X - PS4 Square", "XboxTypeS_X");
    BindingsList.emplace_back("Xbox B - PS4 Circle", "XboxTypeS_B");
    BindingsList.emplace_back("Xbox A - PS4 X", "XboxTypeS_A");
    BindingsList.emplace_back("Xbox LB - PS4 L1", "XboxTypeS_LeftShoulder");
    BindingsList.emplace_back("Xbox RB - PS4 R1", "XboxTypeS_RightShoulder");
    BindingsList.emplace_back("Xbox LT - PS4 L2", "XboxTypeS_LeftTrigger");
    BindingsList.emplace_back("Xbox RT - PS4 R2", "XboxTypeS_RightTrigger");
    BindingsList.emplace_back("Left thumbstick X axis", "XboxTypeS_LeftX");
    BindingsList.emplace_back("Left thumbstick Y axis", "XboxTypeS_LeftY");
    BindingsList.emplace_back("Right thumbstick X axis", "XboxTypeS_RightX");
    BindingsList.emplace_back("Right thumbstick Y axis", "XboxTypeS_RightY");

    UI->EditElement(CVAR_ROLL_BINDING).AddDropdownOptions(BindingsList);
    //UI->EditElement(CVAR_FOV_BINDING).AddDropdownOptions(BindingsList); // #TODO: Uncomment this once FOV smoothing is implemented
}

void CameraManager::SetInputSwapOptions()
{
    UIElement::DropdownOptionsType OptionsList;

    /*
    
        #TODO: FILL LIST HERE - things like forward, right, pitch, etc
        This is the list of inputs people can choose to swap with Roll or FOV
    
    */

    //UI->EditElement(RollSwapsWithWhatInput).AddDropdownOptions(OptionsList);
    //UI->EditElement(FOVSwapsWithWhatInput).AddDropdownOptions(OptionsList);
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

void CameraManager::CacheFOVBinding()
{
    //FOVBindingIndex = GlobalGameWrapper->GetFNameIndexByString(*m_FOVBinding); // #TODO: Uncomment this once FOV smoothing is implemented
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
        RT::Matrix3 MovementMatrix = GetCameraMatrix(*m_bUseLocalMovement);
        RT::Matrix3 RotationMatrix = GetCameraMatrix(*m_bUseLocalRotation);
        UpdateVelocity(Delta, MovementMatrix);
        UpdateAngularVelocity(Delta, RotationMatrix);
        UpdatePosition(Delta, TheCamera);
        UpdateRotation(Delta, TheCamera);
    }
}

void CameraManager::UpdateVelocity(float Delta, RT::Matrix3 InMatrix)
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
    Vector RightAcceleration   = RightInputVec   * GetReducedPerc(RightInputPerc, RightSpeedPerc);
    Vector UpAcceleration      = UpInputVec      * GetReducedPerc(UpInputPerc, UpSpeedPerc);
    
    //Calculate impulse created by inputs
    Vector AccelerationDirection = ForwardAcceleration + RightAcceleration + UpAcceleration;
    Vector Acceleration = AccelerationDirection * ImpulseStrength;

    //Calculate automatic braking force (only apply brakes if no input on that axis)
    Vector ForwardBrake = InMatrix.forward * GetBrakeForce(ForwardInputPerc, ForwardSpeedPerc);
    Vector RightBrake   = InMatrix.right   * GetBrakeForce(RightInputPerc, RightSpeedPerc);
    Vector UpBrake      = InMatrix.up      * GetBrakeForce(UpInputPerc, UpSpeedPerc);
    
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

void CameraManager::UpdateAngularVelocity(float Delta, RT::Matrix3 InMatrix)
{
    //#TODO: Take FOVRotationScale into account here, along with camera FOV

    //RotationSpeed should only be taken into account for Pitch and Yaw if bUsingGamepad is true
    //Roll is set by both keyboard and controller as a rate, along with Pitch and Yaw on controller
    //Pitch and Yaw are set by mouse via movement deltas which give large numbers, so speed should not be used

    //Calculate some variables used throughout the function
    float MaxSpeed = BaseRotationSpeed * *m_RotationSpeed;
    float MouseAccelSpeed = BaseRotationAccel * *m_RotationAccelMouse;
    float GamepadAccelSpeed = BaseRotationAccel * *m_RotationAccelGamepad;
    float GamepadImpulseStrength = MaxSpeed * Delta * GamepadAccelSpeed;

    //Get the camera's angular speed as a percentage per axis
    float PitchSpeedPerc = GetAngularSpeedComponent(InMatrix.right);
    float YawSpeedPerc   = GetAngularSpeedComponent(InMatrix.up);
    float RollSpeedPerc  = GetAngularSpeedComponent(InMatrix.forward);

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
        
    }

    //Convert those inputs into vector inputs
    Vector PitchInputVec = InMatrix.right   * PitchInputPerc;
    Vector YawInputVec   = InMatrix.up      * YawInputPerc;
    Vector RollInputVec  = InMatrix.forward * RollInputPerc;

    //Calculate acceleration force per axis
    Vector PitchAcceleration = PitchInputVec * GetReducedPerc(PitchInputPerc, PitchSpeedPerc);
    Vector YawAcceleration   = YawInputVec   * GetReducedPerc(YawInputPerc,   YawSpeedPerc);
    Vector RollAcceleration  = RollInputVec  * GetReducedPerc(RollInputPerc,  RollSpeedPerc);

    //Calculate impulse created by inputs
    Vector AccelerationDirection = PitchAcceleration + YawAcceleration + RollAcceleration;
    Vector Acceleration = AccelerationDirection * GamepadImpulseStrength;

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

void CameraManager::UpdatePosition(float Delta, CameraWrapper TheCamera)
{
    Vector NewLocation = TheCamera.GetLocation() + Velocity * Delta;

    if(*m_bHardFloors)
    {
        NewLocation.Z = max(NewLocation.Z, *m_FloorHeight);
    }

    TheCamera.SetLocation(NewLocation);
}

void CameraManager::UpdateRotation(float Delta, CameraWrapper TheCamera)
{
    Vector RotationAxis = AngularVelocity.getNormalized();
    float RotationAmount = AngularVelocity.magnitude() * Delta * (CONST_PI_F / 180.f);
    Quat NewRotation = AngleAxisRotation(RotationAmount, RotationAxis);

    RT::Matrix3 CurrentMatrix(TheCamera.GetRotation());
    CurrentMatrix.RotateWithQuat(NewRotation);
    
    TheCamera.SetRotation(CurrentMatrix.ToRotator());
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

RT::Matrix3 CameraManager::GetCameraMatrix(bool bFullyLocal)
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
    Output.forward.Z = 0.f; Output.forward.normalize();
    Output.right.Z   = 0.f; Output.right.normalize();
    Output.up = Vector(0, 0, 1);

    return Output;
}

float CameraManager::GetSpeedComponent(Vector Direction)
{
    float MaxSpeed = BaseMovementSpeed * *m_MovementSpeed;
    return Vector::dot(Velocity, Direction) / MaxSpeed;
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
    return;

    if(!IsValidMode())
    {
        return;
    }

    //Get the camera's speed as a percentage per axis
    RT::Matrix3 MovementMatrix = GetCameraMatrix(*m_bUseLocalMovement);
    RT::Matrix3 RotationMatrix = GetCameraMatrix(*m_bUseLocalRotation);
    float ForwardSpeedPerc = GetSpeedComponent(MovementMatrix.forward);
    float RightSpeedPerc   = GetSpeedComponent(MovementMatrix.right);
    float UpSpeedPerc      = GetSpeedComponent(MovementMatrix.up);
    float PitchSpeedPerc   = GetAngularSpeedComponent(RotationMatrix.right);
    float YawSpeedPerc     = GetAngularSpeedComponent(RotationMatrix.up);
    float RollSpeedPerc    = GetAngularSpeedComponent(RotationMatrix.forward);

    //Angular junk
    Vector RotationAxis = AngularVelocity.getNormalized();
    float RotationAmount = AngularVelocity.magnitude() * 0.008333f * (CONST_PI_F / 180.f);//magic number is roughly the same as Delta in playertick, and converting from deg to rad
    Quat NewRotation = AngleAxisRotation(RotationAmount, RotationAxis);
    
    //Create RenderStrings and fill it with some values
    std::vector<std::string> RenderStrings;
    RenderStrings.push_back("bUsingGamepad: " + std::to_string(Inputs->GetbUsingGamepad()));
    RenderStrings.push_back("Forward: "       + std::to_string(Inputs->GetForward()));
    RenderStrings.push_back("Strafe: "        + std::to_string(Inputs->GetRight()));
    RenderStrings.push_back("Up: "            + std::to_string(Inputs->GetUp()));
    RenderStrings.push_back("Pitch: "         + std::to_string(Inputs->GetPitch()));
    RenderStrings.push_back("Yaw: "           + std::to_string(Inputs->GetYaw()));
    RenderStrings.push_back("Roll: "          + std::to_string(Inputs->GetRoll()));
    RenderStrings.push_back("bRoll: "         + std::to_string(bRoll));
    RenderStrings.emplace_back("");
    RenderStrings.push_back("Total Velocity: "   + CBUtils::PrintFloat(Velocity.magnitude(), 6));
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
