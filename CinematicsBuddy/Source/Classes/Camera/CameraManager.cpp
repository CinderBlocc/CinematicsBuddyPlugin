#include "CameraManager.h"
#include "InputsManager.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"

/*

    #TODO:
        - Separate the camera matrix into two separate ones?
            - One for local location and one for local rotation
            - Sometimes people might want the old "world space" movement, but keep the local rotation

*/

CameraManager::CameraManager()
    :
Inputs(std::make_shared<InputsManager>()),
Graphs(std::make_shared<BMGraphs>(GlobalCvarManager, GlobalGameWrapper)),
bUseOverrides(false),
bUseLocalMatrix(true),
bRoll(false),
MovementSpeed(1.f),
MovementAccel(1.f),
RotationSpeed(1.f),
RotationAccelMouse(1.f),
RotationAccelGamepad(1.f),
MouseSensitivity(10.f),
GamepadSensitivity(20.f),
FOVRotationScale(.9f),
BaseMovementSpeed(2000.f),
BaseMovementAccel(2.f),
BaseRotationSpeed(1.f),
BaseRotationAccel(1.f) {}

void CameraManager::PlayerInputTick(float Delta, bool InbRoll)
{
    if(!bUseOverrides)
    {
        return;
    }

    //Get the inputs and then nullify them before they reach the game
    bRoll = InbRoll;
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
    if(bUseLocalMatrix)
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
    float MaxSpeed = BaseMovementSpeed * MovementSpeed;
    float AccelSpeed = BaseMovementAccel * MovementAccel;
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

    float MaxSpeed = BaseRotationSpeed * RotationSpeed;
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
float CameraManager::GetSpeedComponent(Vector Direction)
{
    float MaxSpeed = BaseMovementSpeed * MovementSpeed;
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
    if(!bUseOverrides)
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


    // TESTS - REMOVE WHEN DONE //
    //Graphs->Render(Canvas);
}
