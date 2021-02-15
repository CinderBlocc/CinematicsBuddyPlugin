#include "CameraManager.h"
#include "InputsManager.h"
#include "SupportFiles/CBUtils.h"
#include "SupportFiles/MacrosStructsEnums.h"

CameraManager::CameraManager()
    :
Inputs(std::make_shared<InputsManager>()),
bUseOverrides(false),
bUseLocalMatrix(true),
bRoll(false),
MovementSpeed(1.f),
MovementAccel(1.f),
RotationAccel(1.f),
MouseSensitivity(10.f),
GamepadSensitivity(20.f),
FOVRotationScale(.9f),
BaseSpeed(2000.f),
MaxSpeed(2000.f) {}

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
    //#TODO: Acceleration/Brake inputs should only be reduced if they continue adding velocity in the current speed direction
    //If they are adding velocity in the opposite direction from the current speed, they should not be reduced at all

    //Calculate the max speed
    float MaxSpeed = BaseSpeed * MovementSpeed;

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

    //Get impulse per axis     
    //Reduced perc should reduce the impulse strength the closer the speed is to max on that axis
    Vector ForwardImpulse = ForwardInputVec * GetReducedPerc(ForwardInputPerc, ForwardSpeedPerc);
    Vector RightImpulse   = RightInputVec   * GetReducedPerc(RightInputPerc, RightSpeedPerc);
    Vector UpImpulse      = UpInputVec      * GetReducedPerc(UpInputPerc, UpSpeedPerc);
    
    //Calculate impulse created by inputs
    float ImpulseStrength = MaxSpeed * Delta * MovementAccel;
    Vector ImpulseDirection = ForwardImpulse + RightImpulse + UpImpulse;
    Vector Impulse = ImpulseDirection * ImpulseStrength;

    //Calculate automatic braking force (only apply brakes if no input on that axis)
    Vector ForwardBrake = Forward * GetBrakeForce(ForwardInputPerc, ForwardSpeedPerc);
    Vector RightBrake   = Right   * GetBrakeForce(RightInputPerc, RightSpeedPerc);
    Vector UpBrake      = Up      * GetBrakeForce(UpInputPerc, UpSpeedPerc);
    
    //Calculate impulse created by braking
    float BrakeStrength = MaxSpeed * Delta * MovementAccel;
    Vector BrakeDirection = ForwardBrake + RightBrake + UpBrake;
    Vector Brake = BrakeDirection * BrakeStrength;

    //Apply the impulses
    Velocity = Velocity + Impulse - Brake;
}

void CameraManager::UpdateAngularVelocity(float Delta)
{
    //#TODO: Take FOVRotationScale into account here, along with camera FOV
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
    return Vector::dot(Velocity, Direction) / MaxSpeed;
}

float CameraManager::GetInvertedPerc(float InPerc)
{
    return (InPerc >= 0.f) ? (1.f - InPerc) : (1.f + InPerc);
}

float CameraManager::GetWeightedPerc(float InPerc)
{
    //#TODO: Some sort of ease-in ease-out function applied to Output
    //Likely a sin wave shifted up and sliced

    //Ease-out only, or ease-in ease-out?

    float NegativeMult = InPerc < 0.f ? -1.f : 1.f;

    //Ease-out
    float EasedPerc = sin((abs(InPerc) * CONST_PI_F) / 2.f);

    //Ease-in ease-out
    //float EasedPerc = -(cos(abs(InPerc) * CONST_PI_F) - 1.f) / 2.f;

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
    RenderStrings.push_back("MovementSpeed: "      + std::to_string(MovementSpeed));
    RenderStrings.push_back("MovementAccel: "      + std::to_string(MovementAccel));
    RenderStrings.push_back("RotationAccel: "      + std::to_string(RotationAccel));
    RenderStrings.push_back("MouseSensitivity: "   + std::to_string(MouseSensitivity));
    RenderStrings.push_back("GamepadSensitivity: " + std::to_string(GamepadSensitivity));
    RenderStrings.push_back("FOVRotationScale: "   + std::to_string(FOVRotationScale));

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
}
